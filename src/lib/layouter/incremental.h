// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "base.h"

#include <vector>
#include <map>

namespace hyrise {
namespace layouter {
/*
 *  This class defines an incremental layouter, based on the
 *  candidate layouter.
 *
 * Based on an initial layout more and more queries can be added
 * and incrementally return a new layout.
 */
class IncrementalCandidateLayouter : public CandidateLayouter {

  std::vector<subset_t> _splittedSets;
  std::vector<subset_t> _unusedPartitions;

  std::vector<double> _unusedCostForPartitions;

  std::map<unsigned, unsigned> _query_conversion;
  std::map<unsigned, unsigned> _reverse_query_conversion;

  void clearState();

  /*
    Based on the affected attribute and the query we build a new
    schema that is used for the incremental layout step.
  */
  Schema buildSchema(subset_t attributes, Query *q);

  /*
    The intermediate result is mapped to the intermediate
    schema, so we need to rewrite the result to match the old
    layout. The cost vector identifies the costs for the queries
    of the new layout. These cost will then be matched against
    the cached cost for the unaffected queries.
  */
  Result rewriteResult(Result affectedBest);

  /*
    Detects all affected partitions from the given query

    The current layout is split into two parts, those partitions
    that are accessed by the query and those which are not
  */
  void detectAffectedPartitions(Query *q);


 public:

  void incrementalLayout(Query *q);

};

} } // namespace layouter

