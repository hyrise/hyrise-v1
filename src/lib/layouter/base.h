// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "config.h"
#include "matrix.h"
#include "layout_utils.h"


#include <iostream>
#include <list>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

namespace hyrise {
namespace layouter {

class Query;


/*
  A Schema is a definition of attributes and queries, for readability
  the schema carries the attribute names
*/
class Schema {
 private:
  std::vector<std::string> attnames;

 public:
  std::vector<Query *> queries;
  size_t nbAttributes;
  size_t nbTuples;

  std::vector<unsigned> attributes;

  Schema() {}

  Schema(std::vector<unsigned> a, int nbA, std::vector<std::string> an);

  void add(const Query *q);

  void removeLastQuery();

  std::vector<std::string> getAttributeNames() const {
    return attnames;
  }

  double costForSubset(subset_t t, std::string costModel) const;

  Schema baseCopy() const;


  void print() {
    std::cout << "<Schema: attrs:" << nbAttributes << " queries:" << queries.size() << ">" << std::endl;
  }

};

/*
  The Query actually defines an operator access on a given schema
*/
class Query {

 public:

  // Maps to the different query types, full projection, out of order, etc..
  LayouterConfiguration::access_type_t type;

  // Attribute of the query
  std::vector<unsigned> queryAttributes;

  // Selectivity if necessary
  double parameter;

  // Weight of the query
  unsigned weight;

 private:

  // Constant std::string value for the cost model
  std::string _costModel;
  // Reference to the schema
  Schema _schema;

  // For log access to the attributes
  std::set<unsigned> _attributeRefSet;

  std::vector<unsigned> _container;
  std::vector<unsigned> _attUnion;
  std::vector<unsigned> _attsOffset;

  unsigned _containerWidth;
  unsigned _previousLine;

 public:

  Query(LayouterConfiguration::access_type_t type, std::vector<unsigned> qA, double parameter, int weight);

  unsigned getContainerWidth();
  void getAttUnion();
  double containerCost(std::vector<unsigned> container, Schema s, std::string costModel);
  double hyriseCost();
  double hyriseProjection();
  double hyrisePartialProjection(unsigned po, unsigned pw);
  double hyriseEquivalentProjection(bool v);
  double hyriseOOO();
  double hyriseSingleOOO(int po, int pw);

};

class Layout {

 public:
  typedef std::vector<subset_t> internal_layout_t;

  explicit Layout(internal_layout_t l);

  Layout();

  void add(std::vector<unsigned> subset);

  bool operator==(const Layout &l) const;

  void removeLast();

  inline size_t containerCount() const {
    return _layout.size();
  }

  inline size_t size() const {
    size_t sum = 0;
    for (subset_t t: _layout) {
      sum += t.size();
    }
    return sum;
  }

  inline bool canAdd(const subset_t &subset) const {
    for (const auto& tmp: _cached_layout) {
      // If it is included in the layout
      for (const auto& i: subset)
          if (tmp.count(i) > 0)
            return false;

      //if (tmp.end() != find_first_of(tmp.begin(), tmp.end(), subset.begin(), subset.end()))
      //return false;
    }
    return true;
  }

  inline void print() const {
    for (const auto& tmp: _layout) {
      std::cout << "|";
      for (const auto& i: tmp) {
        std::cout << i << " ";
      }
      std::cout << "|";
    }
    std::cout << std::endl;
  }

  inline internal_layout_t raw() const {
    return _layout;
  }

 private:

  internal_layout_t _layout;

  // Set based layout type
  typedef std::vector<std::set<unsigned> > _set_based_layout_t;
  _set_based_layout_t _cached_layout;

};


class Result {
  std::vector<std::string> _names;

 public:

  Layout layout;
  std::vector<double> cost;
  double totalCost;

  Result(Layout &l, std::vector<double> c);

  Result() {};

  bool operator==(const Result &r) const;

  bool layoutEquals(const Result &r) const;

  bool operator<(const Result &other) const {
    return totalCost < other.totalCost;
  }

  void print() const;

  void setNames(std::vector<std::string> n) {
    _names = n;
  }

  std::string output() const;

};


/*
  Base class for layouting, this class can be used as a first step
  for column and row layouting.
*/
class BaseLayouter {

 public:

  std::vector<subset_t> subsets;
  int nbLayouts;

  std::vector<Result> results;
  Schema schema;
  std::string costModel;


  // Constructor
  BaseLayouter();

  virtual ~BaseLayouter() {};

  // Some people call it brute force
  virtual void layout(Schema s, std::string costModel);

  std::vector<double> getCost(Layout l);

  std::vector<double> getNewCostForBestLayout(std::string newCostModel);

  std::vector<std::string> getAttributeNames() const;

  std::vector<Result> getNBestResults(size_t n);

  Result getBestResult(std::string newCostModel = "");

  Result getResult(std::string newCostModel, int idx);

  size_t count() {
    return results.size();
  }

  Result findEqualLayout(std::string newCostModel, Result other);

  void generateSubSets();

  std::vector<subset_t> externalGenerateSubSetsK(unsigned k, unsigned nbAtts);

  void generateSubSetsK(unsigned k, unsigned nbAtts);

  void generateSet(std::vector<subset_t> &ctx,
                   subset_t s, unsigned position,
                   unsigned nextInt, unsigned k, unsigned N);

  void iterateThroughLayouts();

  void generateLayouts(Layout l, size_t iter);

  bool tryToAddSubset(const Layout &l, const subset_t &subset) const;

  // Get cost for column layout
  double getColumnCost() const;

  // get cost for row layout
  double getRowCost() const;

  /*
    New and faster iteration method that will yield significant
    performance benefits.  Basically it reduces the problem space of
    iterating on all possible combinations for all different set sizes
    by defining the best possible abortion criteria so that we can
    remove lots of possibilities.

    \param n The current depth of the recursion
    \param list The rest list of all possible subsets that should be combined
    \param current_size The current size of subsets
    \param dest_size The total number of subsets that can be combined
    \param max_attr The maximum number of attributes
    \param curr_attr The current number of attributes
    \return A list of combinations
  */
  std::vector<std::vector<subset_t> > iterateThroughLayoutSubsetsFast(size_t n, std::vector<subset_t> list, size_t current_size, size_t dest_size, size_t max_attr, size_t curr_attr);
    
 protected:

  size_t checkLowerBound(subset_t input, Layout::internal_layout_t subsets);

  void iterateLayoutSubsets(subset_t input, Layout::internal_layout_t subsets);


  Layout::internal_layout_t eliminateInvalidSubsets(subset_t reference, Layout::internal_layout_t input);

};


/*
  Candidate Layouter is the general optimal layouter that does not
  iterate over the complete result set but rather optimizes the
  partitions to check.
*/
class CandidateLayouter: public BaseLayouter {

 protected:

  std::vector< std::set<unsigned> > _candidateList;

  std::vector<subset_t> candidateMergePath(subset_t initial, subset_t rest, std::vector<subset_t> &mapping, std::unordered_map<std::string, double> &_cache);

 public:
  CandidateLayouter();

  virtual ~CandidateLayouter() {};

  virtual void layout(Schema s, std::string costModel);

  void generateCandidateList();

  std::vector<subset_t> externalCombineCandidates(std::vector<std::set< unsigned> > candidateList);

  void combineCandidates();
};


/*
  New Fast Layouter based on monothonic properties of the cost
  functions
*/
class FastCandidateLayouter : public CandidateLayouter {
 protected:

  Layout::internal_layout_t result;
  std::vector<subset_t> _mapping;

  double costFor(subset_t first);
  double costFor(subset_t first, subset_t second);

  void generateResults(subset_t initial, subset_t other);

 public:

  FastCandidateLayouter(): CandidateLayouter()
  {}

  virtual void layout(Schema s, std::string costModel);

};


/*
 * Based on the detected primary partitions build an affinity
 * matrix and from the resulting graph perfrom graph partitioning.
 */
class DivideAndConquerLayouter : public CandidateLayouter {

 protected:

  std::vector<std::vector<set_subset_t> > partitionGraph(Matrix<int> m);

  /*
    Based on the given subsets create an affinity matrix
    representing the count of co accessed attributes for the
    workload
  */
  Matrix<int> createAffinityMatrix();

 public:

  DivideAndConquerLayouter();

  virtual ~DivideAndConquerLayouter() {}

  virtual void layout(Schema s, std::string costModel);

  static int numCuts;

};

} } // namespace hyrise::layouter
