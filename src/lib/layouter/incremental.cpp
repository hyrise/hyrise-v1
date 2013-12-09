// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "base.h"
#include "incremental.h"

#include <boost/foreach.hpp>
#include <map>

namespace hyrise {
namespace layouter {

void IncrementalCandidateLayouter::clearState() {
  _splittedSets.clear();
  _unusedPartitions.clear();

  _query_conversion = std::map<unsigned, unsigned>();
  _reverse_query_conversion = std::map<unsigned, unsigned>();
}

void IncrementalCandidateLayouter::detectAffectedPartitions(Query *q) {
  // Now get all containers and compare to the best original
  // layout
  Result currentBest = getBestResult();
  Layout::internal_layout_t currentLayout = currentBest.layout.raw();

  // Create a set for easier deletion
  set_subset_t current = set_subset_t(q->queryAttributes.begin(), q->queryAttributes.end());

  // Compare the current query set against all others
  for (size_t j = 0; j < currentLayout.size(); ++j) {
    subset_t base = currentLayout[j];

    bool found = false;
    BOOST_FOREACH(unsigned baseAttr, base) {
      BOOST_FOREACH(unsigned queryAttr, current) {
        if (baseAttr == queryAttr) {
          current.erase(queryAttr);
          found = true;
          break; // exit to container
        }
      }

      if (found) {
        _splittedSets.push_back(base);
        break; // exit to next container
      }
    }

    // Capture all unsused partitions
    if (!found) {
      _unusedPartitions.push_back(base);
    }
  }
}

Schema IncrementalCandidateLayouter::buildSchema(subset_t attributes, Query *q) {
  // Modify Schema
  Schema s = schema;
  s.attributes = attributes;
  s.nbAttributes = attributes.size();

  // Add new queries
  s.queries.push_back(q);

  _unusedCostForPartitions = std::vector<double>(s.queries.size(), 0.0);

  // remove all queries that are not used and rewrite the uesed
  // ones to the new layout
  std::vector<Query *> newQueries;
  for (size_t j = 0; j < s.queries.size(); ++j) {
    Query *newQ = new Query(*s.queries[j]);

    // we have to check if this accesses the original
    // attributes
    bool found = false;
    for (size_t i = 0; i < newQ->queryAttributes.size(); ++i) {
      if (_query_conversion.count(newQ->queryAttributes[i]) > 0)
        found = true;

      newQ->queryAttributes[i] = _query_conversion[newQ->queryAttributes[i]];
    }

    if (found)
      newQueries.push_back(newQ);
    else {
      // Cache the cost for the query
      // TODO check if the last query may not be part of this check
      _unusedCostForPartitions[j] = getBestResult().cost[j];
      delete newQ;
    }
  }

  s.queries = newQueries;

  return s;
}

Result IncrementalCandidateLayouter::rewriteResult(Result affectedBest) {
  // Results contain only those parts that were modified, we
  // need to attach the rest. Since we rewrote the original
  // queries and containers, we have to reverse this
  Layout rewritten;

  BOOST_FOREACH(subset_t t, affectedBest.layout.raw()) {
    subset_t newContainer;
    BOOST_FOREACH(unsigned i, t) {
      newContainer.push_back(_reverse_query_conversion[i]);
    }

    rewritten.add(newContainer);
  }

  // Add the unused partitions to the new result layout
  BOOST_FOREACH(subset_t t, _unusedPartitions)
      rewritten.add(t);

  // SInce we cached the cost for all queries (including the new
  // one) we can now insert the modified cost at the slots where
  // the cost are 0 because the query is not part of the cached
  // ones
  for (size_t i = 0, j = 0; i < _unusedCostForPartitions.size(); ++i) {
    if (_unusedCostForPartitions[i] == 0.0) {
      _unusedCostForPartitions[i] = affectedBest.cost[j++];
    }
  }

  // Recalculate cost
  //Result newresult(rewritten, getCost(rewritten));
  Result newresult(rewritten, _unusedCostForPartitions);
  return newresult;
}

void IncrementalCandidateLayouter::incrementalLayout(Query *q) {
  clearState();
  detectAffectedPartitions(q);

  // Perform layout for each of the queries for the layout
  // subset
  subset_t qa;
  unsigned counter = 0;
  BOOST_FOREACH(subset_t i, _splittedSets) {
    BOOST_FOREACH(unsigned j, i) {
      // Width of an attribute
      qa.push_back(4);
      _query_conversion[j] = counter;
      _reverse_query_conversion[counter++] = j;
    }
  }

  // Now we instantiate a new base layouter and find all
  // matching layouts
  CandidateLayouter bl;
  Schema mappedSchema = buildSchema(qa, q);

  bl.layout(mappedSchema, HYRISE_COST);

  // // update my schema etc
  schema.queries.push_back(q);

  // // clear all results
  results.clear();

  // // TODO iterate over all results and add them
  Result mappedBest = bl.getBestResult();
  Result best = rewriteResult(mappedBest);
  results.push_back(best);

}

} } // namespace hyrise::layouter

