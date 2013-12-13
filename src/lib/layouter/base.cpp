// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "base.h"
#include "layout_utils.h"
#include "matrix.h"

#include <algorithm>
#include <float.h>
#include <functional>
#include <limits.h>
#include <math.h>
#include <map>
#include <sstream>
#include <unordered_map>
#include <list>


#include <boost/unordered_set.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/combination.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>


#include <metis.h>

using namespace boost::assign;



namespace std {

template<>
struct hash<hyrise::layouter::subset_t> {
public:

  size_t operator()(const std::vector<unsigned> &ref) const {
    size_t test = 0;
    std::hash<unsigned int> h;
for (auto t : ref)
      test ^= (h(t) << 1);
    return test;
  }
};

} // namespace std

namespace hyrise {
namespace layouter {

Query::Query(LayouterConfiguration::access_type_t type, std::vector<unsigned> qA, double parameter, int weight):
  type(type), queryAttributes(qA), parameter(parameter), weight(weight) {
  std::sort(queryAttributes.begin(), queryAttributes.end());

}

unsigned Query::getContainerWidth() {
  unsigned width = 0;
  BOOST_FOREACH(unsigned a, _container) {
    width += _schema.attributes[a];
  }
  return width;
}

void Query::getAttUnion() {

  // Prepare
  _attributeRefSet.clear();
  _attributeRefSet.insert(queryAttributes.begin(), queryAttributes.end());
  _attUnion.clear();


  // Search
  BOOST_FOREACH(unsigned l, _container)
  if (_attributeRefSet.count(l) > 0)
    _attUnion.push_back(l);

  std::sort(_attUnion.begin(), _attUnion.end());

  if (_costModel.compare(COL_COST) == 0) {
    _attUnion.push_back(0);
  }
}

double Query::containerCost(std::vector<unsigned> container, Schema s, std::string costModel) {
  _container  = container;
  _schema = s;
  _costModel = costModel;

  _containerWidth = getContainerWidth();

  // Check if there are any attributes, that match agains the given containers
  getAttUnion();

  double cost = 0;

  if (_attUnion.size() == 0) {
    return 0;
  }

  if (_costModel.compare(HYRISE_COST) == 0) {
    cost = hyriseCost();
  } else {
    cost = INT_MIN;
  }

  return weight * cost;
}

double Query::hyriseCost() {
  double misses = 0;

  if (type == LayouterConfiguration::access_type_fullprojection) {
    misses = hyriseProjection();
  } else if (type == LayouterConfiguration::access_type_outoforder) {
    misses = hyriseOOO();
  } else {
    misses = DBL_MIN;
  }

  return misses;
}

double Query::hyriseProjection() {
  if (_containerWidth <= CACHE_LINE_SIZE) {
    return ceil(_containerWidth * _schema.nbTuples / (CACHE_LINE_SIZE * 1.0));
  }

  if (_attUnion.size() == 1) {

    unsigned po = 0;

    for (unsigned i = 0; i < _container.size(); ++i) {
      if (_container[i] == _attUnion[0]) {
        break;
      }

      po += _schema.attributes[_container[i]];
    }

    unsigned pw = _schema.attributes[_attUnion[0]];
    return hyrisePartialProjection(po, pw);
  } else {
    return hyriseEquivalentProjection(false);
  }

}

double Query::hyrisePartialProjection(unsigned po, unsigned pw) {
  if (_containerWidth - pw < CACHE_LINE_SIZE) {
    return ceil(_containerWidth * _schema.nbTuples / (CACHE_LINE_SIZE * 1.0));
  }

  double misses = 0.0;
  int nbPossibleOffsets = LCM(CACHE_LINE_SIZE, _containerWidth) / _containerWidth;

  // the cases creating an additional miss are all the cases for which the offset
  // is greater than the transition point
  for (int r = 0; r < nbPossibleOffsets; r++) {
    int lineoffset = ((r * _containerWidth) + po) % CACHE_LINE_SIZE;
    misses += ceil((double)(lineoffset + pw) / CACHE_LINE_SIZE);
  }

  double averageMisses = (double)misses / nbPossibleOffsets;
  misses = ((double)_schema.nbTuples * averageMisses);
  return misses;
}

double Query::hyriseEquivalentProjection(bool isSelective) {

  // compute the offset for each attribute
  int offset = 0;
  std::vector<int> attsOffset(_container.size());

  for (unsigned i = 0; i < _container.size(); i++) {
    attsOffset[i] = offset;
    offset += _schema.attributes[_container[i]];
  }

  int containerWidth = offset;

  // start with the first attribute and try to make equivalent projection
  std::vector<int> eqProjectionOffset;
  std::vector<int> eqProjectionEnd;

  int currentProjection = -1;

  for (unsigned i = 0; i < _container.size(); ++i) {
    // i is the position of the att in the container
    // att is the attribute number corresponding to i
    int att = _container[i];

    // if the attribute is projected, add it to projection
    if (std::find(queryAttributes.begin(), queryAttributes.end(), att) == queryAttributes.end())
      // att is not projected
    {
      continue;
    } else {
      if (currentProjection == -1) {
        // first projection, add it
        currentProjection = 0;
        eqProjectionOffset.push_back(attsOffset[i]);
        eqProjectionEnd.push_back(attsOffset[i] + _schema.attributes[att]);
        continue;
      }

      // new projected attribute; check if the gap is big
      int gap = attsOffset[i] - eqProjectionEnd[currentProjection];

      if (gap < CACHE_LINE_SIZE) {
        // same projection
        eqProjectionEnd[currentProjection] = attsOffset[i] + _schema.attributes[att];
      } else {
        // new projection
        currentProjection++;
        eqProjectionOffset.push_back(attsOffset[i]);
        eqProjectionEnd.push_back(attsOffset[i] + _schema.attributes[att]);
      }
    }
  }

  //compute the gap between the first and last projection
  if (currentProjection > 0) {
    int interRowGap = eqProjectionOffset[0] + containerWidth - eqProjectionEnd[currentProjection];

    if (interRowGap < CACHE_LINE_SIZE) {
      // merge last with first projection
      eqProjectionOffset[0] = eqProjectionOffset[currentProjection];
      eqProjectionEnd[0] = eqProjectionEnd[0] + containerWidth;

      eqProjectionOffset.pop_back();
      eqProjectionEnd.pop_back();
    }

  }

  double misses = 0;

  for (unsigned i = 0; i < eqProjectionOffset.size(); i++) {
    int po = eqProjectionOffset[i];
    int pw = eqProjectionEnd[i] - po;

    if (!isSelective) {
      misses += hyrisePartialProjection(po, pw);
    } else {
      misses += hyriseSingleOOO(po, pw);
    }
  }

  return misses;
}

double Query::hyriseOOO() {

  if (_attUnion.size() == 1) {
    int po = 0;

    for (unsigned i = 0; i < _container.size(); i++) {
      if (_container[i] == _attUnion[0]) {
        break;
      }

      po += _schema.attributes[_container[i]];
    }

    int pw = _schema.attributes[_attUnion[0]];
    return hyriseSingleOOO(po, pw);
  } else {
    return hyriseEquivalentProjection(true);
  }
}

double Query::hyriseSingleOOO(int po, int pw) {
  double misses = 0;

  //int cacheLineTransitionPoint = CACHE_LINE_SIZE - (pw % CACHE_LINE_SIZE) + 1;
  int nbPossibleOffsets = LCM(CACHE_LINE_SIZE, _containerWidth) / _containerWidth;

  for (int r = 0; r < nbPossibleOffsets; r++) {
    int lineoffset = ((r * _containerWidth) + po) % CACHE_LINE_SIZE;
    misses += ceil((double)(lineoffset + pw) / CACHE_LINE_SIZE);
  }

  double averageMisses = (double)misses / nbPossibleOffsets;
  misses = ((double)_schema.nbTuples * averageMisses);

  // try to divide in the traditional two cases
  if (_containerWidth - pw < CACHE_LINE_SIZE) {
    // compute the average number of rows in a cache line
    double nbRowsInLine = ((double)(CACHE_LINE_SIZE)) / _containerWidth;

    // compute the average number of lines that are touched for each projection
    double rowsToSubstract = 1 + (nbRowsInLine - 1) * parameter;

    misses = (averageMisses * ((double)_schema.nbTuples * parameter / rowsToSubstract));
  } else {
    // "simple" case, rows are independent w.r.t. misses
    misses = (int)(averageMisses * ((double)_schema.nbTuples * parameter));
  }

  return misses;
}

Layout::Layout(internal_layout_t l): _layout(l) {
}

Layout::Layout(): _layout() {
}

bool Layout::operator==(const Layout &l) const {
  if (_layout.size() == 0) {
    return false;
  }

  for (unsigned i = 0; i < l._layout.size(); ++i) {
    std::vector<unsigned> left = _layout[i];
    std::vector<unsigned> right = l._layout[i];

    if (left.size() != right.size()) {
      return false;
    }

    for (unsigned j = 0; j < left.size(); ++j) {
      if (left[j] != right[j]) {
        return false;
      }
    }
  }

  return true;
}

void Layout::add(std::vector<unsigned> subset) {
  _layout.push_back(subset);

  // Set based caching of layouts
  std::set<unsigned> tmp(subset.begin(), subset.end());
  _cached_layout.push_back(tmp);
}

void Layout::removeLast() {
  _layout.pop_back();
}

Result::Result(Layout &l, std::vector<double> c) {
  layout = Layout(l);
  cost = c;
  totalCost = 0;

  for (unsigned i = 0; i < cost.size(); ++i) {
    totalCost += cost[i];
  }
}

bool Result::operator==(const Result &r) const {
  return totalCost == r.totalCost;
}

bool Result::layoutEquals(const Result &r) const {
  return layout == r.layout;
}

void Result::print() const {
  BOOST_FOREACH(subset_t s, layout.raw()) {
    std::cout << "|";
    BOOST_FOREACH(unsigned t, s) {
      std::cout << " " << t << " ";
    }
    std::cout << "|";
  }

  std::cout << " Cost: " << totalCost << " [";
  BOOST_FOREACH(double d, cost) {
    std::cout << " " << d;
  }
  std::cout << " ]" << std::endl;
}

std::string Result::output() const {
  std::vector<std::string> names, types, groups;
  std::stringstream output;
  int counter = 0;
  BOOST_FOREACH(subset_t s, layout.raw()) {
    BOOST_FOREACH(unsigned t, s) {
      // Add the name for each group
      names.push_back(_names[t]);
      types.push_back("INTEGER");
      groups.push_back(str(boost::format("%1%_R") % counter));
    }
    counter++;
  }
  // Finalizing the output
  output << boost::algorithm::join(names, " | ") << std::endl;
  output << boost::algorithm::join(types, " | ") << std::endl;
  output << boost::algorithm::join(groups, " | ") << std::endl;
  return output.str();
}


Schema::Schema(std::vector<unsigned> a, int nbA, std::vector<std::string> an): attnames(an), nbAttributes(a.size()), nbTuples(nbA), attributes(a) {
}

void Schema::add(const Query *q) {
  queries.push_back(new Query(*q));
}

void Schema::removeLastQuery() {
  queries.pop_back();
}

double Schema::costForSubset(subset_t t, std::string costModel) const {
  size_t num_queries = queries.size();
  double total = 0.0;

  for (size_t q = 0; q < num_queries; ++q)
    total += queries[q]->containerCost(t, *this, costModel);

  return total;
}

Schema Schema::baseCopy() const {
  Schema result(*this);
  result.queries.clear();
  return result;
}

BaseLayouter::BaseLayouter(): nbLayouts(0) {
}

void BaseLayouter::layout(Schema s, std::string cM) {
  schema = s;
  costModel = cM;

  generateSubSets();


  iterateThroughLayouts();
  std::sort(results.begin(), results.end());
}


Layout::internal_layout_t BaseLayouter::eliminateInvalidSubsets(subset_t reference, Layout::internal_layout_t input) {
  Layout::internal_layout_t result;
  bool found;
  std::set<unsigned> initial(reference.begin(), reference.end());

  BOOST_FOREACH(subset_t right, input) {
    found = false;
    BOOST_FOREACH(unsigned i, right) {
      if (initial.count(i) > 0) {
        found = true;
        break;
      }
    }
    if (!found)
      result += right;
  }

  return result;
}

void BaseLayouter::iterateLayoutSubsets(subset_t input, Layout::internal_layout_t subsets) {
  subset_t front = input;
  Layout::internal_layout_t rest = subsets;

  // Eliminate all invalid combinations from rest
  Layout::internal_layout_t eliminated = eliminateInvalidSubsets(front, rest);

  std::vector<uint32_t> elimSizes;
for (subset_t t : eliminated)
    elimSizes.push_back(t.size());

  unsigned upperBound = eliminated.size() < schema.nbAttributes ? eliminated.size() : schema.nbAttributes;

  // always check lower bound
  // the lower bound defines how many partitions we need to actually make a valid layout
  unsigned lowerBound = checkLowerBound(input, subsets);

  // std::cout << "---- E / U / L ----" << std::endl;
  print(input);
  print(eliminated);
  return;
  // std::cout << eliminated.size() << std::endl;
  // std::cout << schema.nbAttributes << std::endl;
  // std::cout << upperBound << std::endl;
  // std::cout << lowerBound << std::endl;
  // std::cout << "--------------------" << std::endl;

  for (unsigned i = lowerBound - 1; i <= upperBound; ++i) {

    // Prepare intial setting, since we cannot swap vectors
    subset_t combi;
    for (unsigned j = 0; j < eliminated.size(); ++j)
      combi += j;

    //std::cout << "Combinations " << eliminated.size() << " over " << i << std::endl;
    size_t combinations  = 0;
    size_t noAttr = 0;
    do {
      size_t nb = 0;
      nb += input.size();

      // Check
      for (unsigned j = 0; j < i; ++j) {
        nb += elimSizes[combi[j]];
        if (nb > schema.nbAttributes) {
          ++noAttr;
          goto contLoop;
        }
      }

      // Add front to layout and all other combinations here and check if they are valid
      {
        Layout l;
        l.add(input);

        for (unsigned j = 0; j < i; ++j) {
          if (l.canAdd(eliminated[combi[j]])) {
            l.add(eliminated[combi[j]]);
          } else {
            goto contLoop;
          }
        }

        //std::cout << "New layout?" << std::endl;
        if (nb == schema.nbAttributes) {
          Result r(l, getCost(l));
          nbLayouts++;
          results.push_back(r);
        }
      }
contLoop:
      ++combinations;
    } while (boost::next_combination(combi.begin(), combi.begin() + i, combi.end()));

    //std::cout << combinations << " / " << noAttr << std::endl;

    // if all combinations had more attributes than we can add to the table we can break the loop
    // here since there will be no case where adding a partition will have std::less attributes
    if (combinations == noAttr) {
      break;
    }
  }
}

size_t BaseLayouter::checkLowerBound(subset_t input, Layout::internal_layout_t subsets) {
  // sort subsets by size descending
  Layout::internal_layout_t sorted(subsets);
  sort(sorted.begin(), sorted.end(), sort_subset_by_size);

  unsigned counted = input.size();
  unsigned lowerBound = 1;
  for (size_t i = 0; i < sorted.size(); ++i) {
    counted += sorted[i].size();
    if (counted <= schema.nbAttributes)
      ++lowerBound;
    else
      break;
  }

  if (counted < schema.nbAttributes)
    return std::numeric_limits<unsigned>::max();

  return lowerBound;
}

std::vector<std::vector<subset_t> > BaseLayouter::iterateThroughLayoutSubsetsFast(size_t n,
    std::vector<subset_t> list,
    size_t current_size,
    size_t dest_size,
    size_t max_attr,
    size_t curr_attr) {
//    std::cout << "Recurse with " << n << " " << current_size << " " << dest_size << " " << max_attr << " " << curr_attr << std::endl;
  typedef std::vector<std::vector<subset_t> > iterate_result_t;
  // Recursion abortion defined here
  if (n == 1 or list.size() == 1) {
    iterate_result_t res;
    for (auto x : list) {
      if (x.size() + curr_attr == max_attr) {
        std::vector<subset_t> tmp = {x};
        res.push_back(tmp);
      }
    }
    return res;
  }

  // No we handle the main part of the recursion
  size_t index = 0;
  iterate_result_t result;
  while (index < list.size()) {

    // FXIME
    //if (list.size() - 1 < n - 1 or list.size() == index+1 or list.size() - index < dest_size - current_size)
    //    return result;

    // Check if it makes sense to proceed with recursion Potential
    // improvement. Make sure, we only extract those subsets that are
    // relevant.

    // There is a second case when we can abort: If we know, that
    // we have n-1 elements left and the minimal length is
    // e.g. two that means we generate at lest (n-1)*two
    // attributes, if this number is too large we will abort

    subset_t left_list = list[index];
    std::vector<subset_t> rest_list(list.begin() + index + 1, list.end());

    /* We define another abort criterion here, if a single element
    from our current pivot element is found inside another element
    from the drill down list we can safeley remove these elements
    from the list
    */
    std::vector<subset_t> new_rest;
    for (auto x : rest_list) {
      for (auto f : left_list)
        for (auto r : x)
          if (f == r)
            goto bbb;
          
      new_rest.push_back(x);

      bbb:
      continue;
    }
    
    rest_list = new_rest;
    // rest_list.erase(std::remove_if(rest_list.begin(), rest_list.end(), [&left_list](subset_t& e){
    //             for( const auto& f : left_list)
    //                 if (std::find(e.begin(), e.end(), f) != left_list.end())
    //                     return true;

    //             return false;
    //         }), rest_list.end());

    iterate_result_t all;

    if (rest_list.size() > 0 &&
      curr_attr + rest_list[0].size() <= max_attr &&
      (rest_list[0].size() * (n - 1) + curr_attr <= max_attr or list.size() < n)) {
      all = iterateThroughLayoutSubsetsFast(n - 1,
        rest_list,
        current_size + 1,
        dest_size,
        max_attr,
        curr_attr + list[index].size());
  }
  
  for (auto part : all) {
      /*
        Now we need a fast way to identify that we will not have
        a single elemnt doubled.
       */
        std::vector<subset_t> tmp {list[index]};
        tmp.insert(tmp.end(), part.begin(), part.end());
        result.push_back(tmp);
      }
      ++index;
    }
    return result;
  }
  
void BaseLayouter::iterateThroughLayouts() {
  std::sort(subsets.begin(), subsets.end(), subset_t_lt);
  
  if (subsets.size() == 1) {
    Layout l;
    l.add(subsets[0]);
    Result r(l, getCost(l));
    nbLayouts++;
    results.push_back(r);
    return;
  }

  //  if (subsets.size() > schema.nbAttributes)
  if (true) {

    // We have an issue here since we do not check what the minimum
    // size of attribute groups is we need to have. If we have the
    // case that only using 100 groups in our 100 attribute group
    // large table we can succeed, we will check all other
    // possibilities before, thus we need to increase the start point
    // to a reasonable amount

    // Since we know the subsets are orered in size, we can deduce the
    // minium amount by the size of the biggest element
    auto start_min = schema.nbAttributes / subsets.back().size();

    for (size_t i = start_min; i <= schema.nbAttributes; ++i) {
      auto intermediate_results = iterateThroughLayoutSubsetsFast(i,
                                  subsets,
                                  0,
                                  i,
                                  schema.nbAttributes,
                                  0);

      for (auto x : intermediate_results) {
        Layout l;
        for (auto t : x)
          l.add(t);
        Result r(l, getCost(l));
        nbLayouts++;
        results.push_back(r);
      }
    }
  } else {

    //list<subset_t> down(subsets.begin(), subsets.end());

    //Layout l;
    //    iterateThroughLayoutSubsets2(l, down);

    for (size_t s = 0; s < subsets.size(); ++s) {
      iterateLayoutSubsets(subsets[s], Layout::internal_layout_t(subsets.begin() + 1 + s, subsets.end()));
    }
  }
}

void BaseLayouter::generateLayouts(Layout layout, size_t iter) {
  // only continue if we are save
  if (iter >= subsets.size()) {
    if (layout.size() == schema.nbAttributes) {
      Result r(layout, getCost(layout));
      if (!(r.layout == results.back().layout)) {
        nbLayouts++;
        results.push_back(r);
      }
    }

    return;
  }

  // get current subset
  subset_t currentSubset = subsets[iter];
  Layout original = layout;

  if (tryToAddSubset(layout, currentSubset)) {
    // Add to the layout and check
    layout.add(currentSubset);
    if (layout.size() == schema.nbAttributes) {
      Result r(layout, getCost(layout));
      nbLayouts++;
      results.push_back(r);
    }

    size_t newIter = iter + 1;
    generateLayouts(layout, newIter);
  }

  if (subsets[iter + 1].size() + original.size() <= schema.nbAttributes)
    generateLayouts(original, iter + 1);
}

bool BaseLayouter::tryToAddSubset(const Layout &l, const subset_t &subset) const {
  if (l.size() + subset.size() > schema.nbAttributes)
    return false;


  if (!l.canAdd(subset))
    return false;

  return true;
}



void BaseLayouter::generateSubSets() {
  for (unsigned i = 1; i <= schema.nbAttributes; i++) {
    generateSubSetsK(i, schema.nbAttributes);
  }
}

std::vector<subset_t> BaseLayouter::externalGenerateSubSetsK(unsigned k, unsigned nbAtts) {
  subset_t s(nbAtts);
  for (size_t i = 0; i < nbAtts; ++i)
    s[i] = i;

  std::vector<subset_t> result;

  do {
    result.push_back(subset_t(s.begin(), s.begin() + k));
  } while (boost::next_partial_permutation(s.begin(), s.begin() + k, s.end()));

  return result;
}

void BaseLayouter::generateSubSetsK(unsigned k, unsigned nbAtts) {
  subsets += externalGenerateSubSetsK(k, nbAtts);
}


void BaseLayouter::generateSet(std::vector<subset_t> &ctx, std::vector<unsigned> s, unsigned position, unsigned nextInt, unsigned k, unsigned N) {
  if (position == k) {
    // create the subset
    subset_t newSubSet(k);

    for (unsigned i = 0; i < k; i++) {
      newSubSet[i] = s[i];
    }

    ctx.push_back(newSubSet);
    return;
  }

  for (unsigned i = nextInt; i < N; i++) {
    s[position] = i;
    generateSet(ctx, s, position + 1, i + 1, k, N);
  }
}

std::vector<double> BaseLayouter::getCost(Layout l) {
  size_t num_queries = schema.queries.size();
  std::vector<double> totalCost(num_queries, 0.0);

  BOOST_FOREACH(subset_t container, l.raw()) {
    for (size_t q = 0; q < num_queries; ++q) {
      totalCost[q] += schema.queries[q]->containerCost(container, schema, costModel);
    }
  }
  return totalCost;
}

std::vector<Result> BaseLayouter::getNBestResults(size_t n) {
  size_t ms = n < results.size() ? n : results.size();
  std::vector<Result> rs = std::vector<Result>(ms);
  for (size_t i = 0; i < ms; ++i) {
    rs[i] = results[i];
    rs[i].setNames(schema.getAttributeNames());
  }
  return rs;
}

Result BaseLayouter::getBestResult(std::string newCostModel) {
  Result r = results.front();
  r.setNames(schema.getAttributeNames());
  return r;
}

std::vector<std::string> BaseLayouter::getAttributeNames() const {
  return schema.getAttributeNames();
}

double BaseLayouter::getRowCost() const {
  // build layout
  subset_t tmp;
  for (size_t i = 0; i < schema.nbAttributes; ++i)
    tmp.push_back(i);

  return schema.costForSubset(tmp, costModel);
}

double BaseLayouter::getColumnCost() const {
  double result = 0.0;
  for (size_t i = 0; i < schema.nbAttributes; ++i) {
    subset_t tmp;
    tmp.push_back(i);
    result += schema.costForSubset(tmp, costModel);
  }

  return result;
}


void FastCandidateLayouter::layout(Schema s, std::string costModel) {
  results.clear();
  subsets.clear();
  _mapping.clear();

  _candidateList = std::vector<std::set<unsigned> >();

  schema = s;
  costModel = costModel;

  // This is the part where the relevant subsets
  // are created
  generateCandidateList();


  // Create subset mapping for primary partitions
  for (unsigned i = 0; i < _candidateList.size(); ++i) {
    _mapping.push_back(subset_t(_candidateList[i].begin(), _candidateList[i].end()));
  }

  // Now Create the required partitions table
  subset_t partitions(_candidateList.size() - 1, 0);
  for (unsigned i = 1; i < _candidateList.size(); ++i)
    partitions[i - 1] = i;

  subset_t initial(1, 0);
  generateResults(initial, partitions);

  // remap result, based on partitions
  std::vector<subset_t> mappedResult;
  BOOST_FOREACH(subset_t s, result) {
    subset_t newSubset;
    BOOST_FOREACH(unsigned i, s) {
      BOOST_FOREACH(unsigned j, _mapping[i])
      newSubset += j;
    }
    mappedResult += newSubset;
  }

  result = mappedResult;

  // Prepare result
  std::vector<double> costs;
  BOOST_FOREACH(subset_t s, result) {
    costs.push_back(schema.costForSubset(s, HYRISE_COST));
  }
  Layout l(result);
  Result r(l, costs);
  results.push_back(r);

}

double FastCandidateLayouter::costFor(subset_t first) {
  double result = 0;
  subset_t tmp;
  BOOST_FOREACH(unsigned i, first)
  BOOST_FOREACH(unsigned j, _mapping[i])
  tmp.push_back(j);

  result = schema.costForSubset(tmp, HYRISE_COST);
  return result;
}

double FastCandidateLayouter::costFor(subset_t first, subset_t second) {
  return costFor(first) + costFor(second);
}

void FastCandidateLayouter::generateResults(subset_t initial, subset_t other) {
  Layout::internal_layout_t intermediate;
  subset_t next;
  double min = std::numeric_limits<double>::max();

  if (other.size() == 0 && initial.size() == 1) {
    result += initial;
    return;
  }

  do {

    subset_t combined(initial);

    combined += other.front();

    // Now we compare the cost for all possibilities, both, merged

    double singleCost = costFor(initial, subset_t(1, other.front()));
    double mergedCost = costFor(combined);

    print(combined);
    std::cout << "--" << std::endl;
    print(initial);
    print(subset_t(1, other.front()));
    std::cout << costFor(initial) << " " << costFor(subset_t(1, other.front())) << std::endl;
    std::cout << "SC " << singleCost << " MC " << mergedCost << std::endl;

    if (singleCost < min) {
      intermediate.clear();
      intermediate += initial, subset_t(1, other.front());

      next = subset_t(other.begin() + 1, other.end());
      min = singleCost;
    }

    if (mergedCost < min) {
      intermediate.clear();
      intermediate += combined;

      next = subset_t(other.begin() + 1, other.end());
      min = mergedCost;
    }

  } while (boost::next_partial_permutation(other.begin(), other.begin() + 1,  other.end()));

  std::cout << "Finished calculation, checking for result " << intermediate.size() << std::endl;

  // The single cost are better, proceed separate
  if (intermediate.size() > 1) {
    result += intermediate.front();
    if (other.begin() + 1 != other.end()) {
      print(intermediate.back());
      print(next);
      generateResults(intermediate.back(), next);
    } else
      result += intermediate.back();
  } else {
    if (other.begin() + 1 != other.end())
      generateResults(intermediate.front(), next);
    else
      result += intermediate.front();
  }
}


CandidateLayouter::CandidateLayouter(): BaseLayouter(), _candidateList() {
}

void CandidateLayouter::layout(Schema s, std::string cM) {
  results.clear();
  subsets.clear();
  _candidateList = std::vector<std::set<unsigned> >();

  schema = s;
  costModel = cM;

  // This is the part where the relevant subsets
  // are created
  generateCandidateList();
  combineCandidates();

  // Finish the job and search for the perfect match
  std::sort(subsets.begin(), subsets.end(), subset_t_lt);

  // Initialize bit vector with all valid subsets, initially all
  // subsets are valid
  std::vector<bool> validSubsets(subsets.size(), true);
  std::vector<bool> uncheckedSubsets(subsets.size(), true);

  // Cache cost map
  std::unordered_map<subset_t, double> cache;


  // Reduce the number of subsets to examine for building the final
  // layout. The main idea in this sub-step is to identify all those
  // merged sub-groups that yield equal cost
  size_t index = 0;
  while (index < subsets.size()) {
    // if the subsets are smaller than a single attribute, ignore
    if (subsets[index].size() > 1) {
      size_t currentSize = subsets[index].size();

      // Find end of group
      size_t stop = index+1;
      while (stop < subsets.size())
        if (subsets[stop++].size() > currentSize)
          break;

      // Now for each element inside this block, find possible partners
      for (size_t i = index; i < stop; ++i) {
        if (!uncheckedSubsets[i])
          continue;


        double current;

        if (cache.count(subsets[i]) == 0)
          cache[subsets[i]] = schema.costForSubset(subsets[i], HYRISE_COST);

        current = cache[subsets[i]];

        // Now compare with all other subsets
        for (size_t j = i + 1; j < stop; ++j) {
          if (uncheckedSubsets[j] && subset_t_content_equal(subsets[i], subsets[j])) {
            uncheckedSubsets[j] = false;
            double test;
            if (cache.count(subsets[j]) == 0)
              cache[subsets[j]] = schema.costForSubset(subsets[j], HYRISE_COST);

            test = cache[subsets[j]];


            if (current == test)
              validSubsets[j] = 0;
          }
        }

      }
    }
    ++index;
  }

  std::vector<subset_t> newList;
  for (size_t i = 0; i < validSubsets.size(); ++i) {
    if (validSubsets[i])
      newList.push_back(subsets[i]);
  }

  subsets = newList;
  iterateThroughLayouts();
  std::sort(results.begin(), results.end());
}

void CandidateLayouter::generateCandidateList() {
  BOOST_FOREACH(Query * q, schema.queries) {
    std::set<unsigned> newQuery;
    BOOST_FOREACH(unsigned i, q->queryAttributes) {
      newQuery.insert(i);
    }

    if (_candidateList.size() == 0) {
      _candidateList.push_back(newQuery);
    } else {
      std::vector< std::set<unsigned> > newSets;
      BOOST_FOREACH(std::set<unsigned> &oldSet, _candidateList) {
        std::set<unsigned> intersect;
        BOOST_FOREACH(unsigned i, newQuery) {
          if (oldSet.count(i) > 0) {
            // the new set and the existing set intersect
            // add the attribute to the intersection
            intersect.insert(i);
          }
        }

        BOOST_FOREACH(unsigned i, intersect) {
          newQuery.erase(i);
          oldSet.erase(i);
        }

        // keep track of which sets to add
        newSets.push_back(intersect);
      }

      // add new sets, i.e. add query and intersect
      _candidateList.push_back(newQuery);
      BOOST_FOREACH(std::set<unsigned> newSet, newSets) {
        _candidateList.push_back(newSet);
      }

      // Delete empty sets
      std::vector<unsigned> toDelete;

      for (size_t i = 0; i < _candidateList.size(); ++i) {
        if (_candidateList[i].size() == 0) {
          toDelete.push_back(i);
        }
      }

      for (size_t i = toDelete.size(); i > 0; --i) {
        _candidateList.erase(_candidateList.begin() + toDelete[i - 1]);
      }
    }
  }

  // add a set for those attributes that are never queried
  std::set<unsigned> neverQueried;

  for (unsigned i = 0; i < schema.nbAttributes; ++i) {
    bool queried = false;
    BOOST_FOREACH(std::set<unsigned> set, _candidateList) {
      if (set.count(i) > 0) {
        queried = true;
        break;
      }
    }

    if (!queried) {
      neverQueried.insert(i);
    }
  }

  if (neverQueried.size() > 0) {
    _candidateList.push_back(neverQueried);
  }
}


std::string strval(subset_t m) {
  std::stringstream strs;
  BOOST_FOREACH(unsigned i, m) {
    strs << i << " ";
  }

  return strs.str();
}


subset_t merge_subsets(subset_t base, std::vector<subset_t> &mapping) {
  subset_t result;
  BOOST_FOREACH(unsigned i, base) {
    BOOST_FOREACH(unsigned j, mapping[i]) {
      result.push_back(j);
    }
  }

  return result;
}

typedef std::unordered_map<std::string, double> cache_t;

// Intermediate Result Structure to capture cost and mappings
struct _intermediate {
  std::vector<subset_t> single;
  double single_cost;
  subset_t merged;
  double merge_cost;

  static _intermediate create(subset_t base, std::vector<subset_t> &mapping, Schema &schema, std::string &costModel, cache_t &cache) {
    _intermediate res;
    res.single_cost = 0.0;
    res.merge_cost = 0.0;

    std::string tmp_val;

    BOOST_FOREACH(unsigned i, base) {
      res.single.push_back(mapping[i]);
      tmp_val = strval(mapping[i]);
      if (cache.count(tmp_val) == 0) {
        cache[tmp_val] = schema.costForSubset(mapping[i], costModel);
      }

      res.single_cost += cache[tmp_val];
      //res.single_cost += schema.costForSubset(mapping[i], costModel);

      BOOST_FOREACH(unsigned j, mapping[i]) {
        res.merged.push_back(j);
      }
    }

    res.merge_cost = schema.costForSubset(res.merged, costModel);
    return res;
  }

};


std::vector<subset_t> CandidateLayouter::externalCombineCandidates(std::vector<std::set< unsigned> > candidateList) {

  std::vector<subset_t> mapping;
  for (unsigned i = 0; i < candidateList.size(); ++i) {
    mapping.push_back(subset_t(candidateList[i].begin(), candidateList[i].end()));
  }


  // Cache to avoid frequent recalculations

  cache_t _cache;

  //The first step is to build a list of permutations of the input
  //candidate list, so that if the input is [1][2,3] -> the output
  //will be [1], [2,3], [1,2,3]
  std::vector<subset_t> result;

  subset_t tmp(candidateList.size(), 0);
  subset_t initial;

  // Prepare the current permutation step
  for (unsigned j = 0; j < candidateList.size(); ++j)
    tmp[j] = j;

  result = candidateMergePath(initial, tmp, mapping, _cache);

  return result;
}

std::vector<subset_t> CandidateLayouter::candidateMergePath(subset_t initial, subset_t rest, std::vector<subset_t> &mapping, std::unordered_map<std::string, double> &_cache) {
  std::vector<subset_t> result;
  do {

    subset_t combined(initial);
    combined.push_back(rest.front());

    _intermediate c = _intermediate::create(combined, mapping, schema, costModel, _cache);
    if (c.single.size() == 1 || c.merge_cost < c.single_cost) {
      // Append the intermediate merged list to the
      result.push_back(c.merged);

      if (rest.begin() + 1 != rest.end()) {
        subset_t down(rest.begin() + 1, rest.end());
        std::vector<subset_t> tmp = candidateMergePath(combined, down, mapping, _cache);
        BOOST_FOREACH(subset_t t, tmp)
        result.push_back(t);
      }
    }
  } while (boost::next_partial_permutation(rest.begin(), rest.begin() + 1, rest.end()));

  return result;
}


void CandidateLayouter::combineCandidates() {
  subsets = externalCombineCandidates(_candidateList);
}

/////////////////////////////////////////////////////////////////////////

int layouter::DivideAndConquerLayouter::numCuts = 4;

DivideAndConquerLayouter::DivideAndConquerLayouter(): CandidateLayouter() {
}


Matrix<int> DivideAndConquerLayouter::createAffinityMatrix() {

  // Build the initial empty matrix
  Matrix<int> d(subsets.size());

  // Now make the big check
  for (size_t i = 0; i < subsets.size() - 1; ++i) {
    subset_t subsetToCheck = subsets[i];

    for (size_t j = i + 1; j < subsets.size(); ++j) {
      subset_t otherSubset = subsets[j];

      // Now check both subsets if they appear in the queries together
      BOOST_FOREACH(Query * q, schema. queries) {

        subset_t::iterator l = std::find_first_of(q->queryAttributes.begin(),
                               q->queryAttributes.end(),
                               subsetToCheck.begin(),
                               subsetToCheck.end());

        subset_t::iterator r = std::find_first_of(q->queryAttributes.begin(),
                               q->queryAttributes.end(),
                               otherSubset.begin(),
                               otherSubset.end());

        if (l != q->queryAttributes.end() && r != q->queryAttributes.end()) {
          // Found a hit
          int tmp = d.get(i, j) + 1;
          d.set(j, i, tmp);
          d.set(i, j, tmp);
        }
      }
    }
  }

  return d;
}

std::vector<std::vector<set_subset_t> > DivideAndConquerLayouter::partitionGraph(Matrix<int> m) {
  adj_t t = m.buildAdjacency();

  int numvertices = m.numVertices();

  int options[METIS_NOPTIONS];
  METIS_SetDefaultOptions(options);
  options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY;
  options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
  options[METIS_OPTION_NUMBERING] = 0;
  //options[METIS_OPTION_DBGLVL] = METIS_DBG_INFO | METIS_DBG_MEMORY | METIS_DBG_COARSEN;
  options[METIS_OPTION_DBGLVL] = 0;

  int ncon = 1;

  // K defines the size of the parition, while k is the number of cuts the
  // partitioner will have to do, so we can calculate k by
  // dividing P/K
  int nparts = numvertices > numCuts ? numvertices / numCuts : 1; // - 1 < numCuts ? numvertices - 1 : numCuts;
  if (nparts == 1)
    nparts = 2;

  int edgecut = 0;

  int *part = (int *) malloc(1 * numvertices * sizeof(numvertices));
  int res = METIS_PartGraphKway(&numvertices,  // Number of Vertices
                                &ncon,  // Number of balancing constraints
                                t.xadj,
                                t.adjncy,
                                nullptr, // vertices weight
                                nullptr, // vertices size
                                t.adjwgt, // edege weight
                                &nparts, // number of parts to partition
                                nullptr, // targeted partition weight
                                nullptr, // ubvec, load imbalance bla bla
                                options,
                                &edgecut,
                                part); // result parts


  if (res == METIS_OK) {
    // std::cout << nparts << std::endl;

    // Based on the partitions of the graph, we can now
    // merge the subsets to new subsets
    std::vector<std::vector<set_subset_t> > candidatePartitions(nparts); // Create at least nparts buckets
    for (int i = 0; i < numvertices; ++i) {
      int current = part[i];
      set_subset_t oldsubset = set_subset_t(subsets[i].begin(), subsets[i].end());

      // Copy the subsets over
      candidatePartitions[current].push_back(oldsubset);
    }

    // the result may contain empty partitions
    return candidatePartitions;

  } else {
    throw std::runtime_error("Graph Partitioning Seriously Failed");
  }
}

void DivideAndConquerLayouter::layout(Schema s, std::string cm) {
  results.clear();
  subsets.clear();

  if (s.queries.size() == 1) {
    CandidateLayouter::layout(s, cm);
    return;
  }

  _candidateList = std::vector<std::set<unsigned> >();

  schema = s;
  costModel = cm;

  // This is the part where the relevant subsets
  // are created
  generateCandidateList();
  subsets.clear();

  BOOST_FOREACH(std::set<unsigned> l, _candidateList) {
    subsets.push_back(subset_t(l.begin(), l.end()));
  }


  Matrix<int> m = createAffinityMatrix();

  // The result of the partitioning operation is a list of parition
  // lists that are layouted independently
  std::vector<std::vector<set_subset_t> > partitions = partitionGraph(m);

  //The partitions variable holds a list of subset lists, which are
  //layouted individually.
  subsets.clear();

  size_t nbAtt = schema.nbAttributes;
  std::vector<subset_t> global;
  BOOST_FOREACH(std::vector<set_subset_t> tmp, partitions) {

    // Ignore empty sets
    if (tmp.size() == 0)
      continue;

    std::vector<subset_t> permutations = externalCombineCandidates(tmp);
    std::set<unsigned> current;
for (subset_t t : permutations)
for (unsigned u: t)
        current.insert(u);

    schema.nbAttributes = current.size();
    subsets = permutations;

    results.clear();
    iterateThroughLayouts();
    std::sort(results.begin(), results.end());

    Result r = results[0];
    Layout l = r.layout;

for (subset_t t : l.raw())
      global += t;
  }

  schema.nbAttributes = nbAtt;
  subsets = global;
  // Finish the job and search for the perfect match
  iterateThroughLayouts();
  std::sort(results.begin(), results.end());
}

} } // namespace hyrise::layouter

