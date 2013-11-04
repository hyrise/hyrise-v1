// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_GROUPBYSCAN_H_
#define SRC_LIB_ACCESS_GROUPBYSCAN_H_

#include "access/system/ParallelizablePlanOperation.h"
#include "access/AggregateFunctions.h"

namespace hyrise {
namespace access {

class GroupByScan : public ParallelizablePlanOperation {
public:
  virtual ~GroupByScan();

  void setupPlanOperation();
  void executePlanOperation();
  /// Reacts to
  /// fields in either integer-list notation or std::string-list notation
  /// functions as a list of {"type": (int|str), "field": (int|str)
  /// Use the GroupByScan always in conjunction with a HashBuild:
  /// {
  ///     "operators": {
  ///         "0": {
  ///              "type": "TableLoad",
  ///              "table": "table1",
  ///              "filename": "..."
  ///          },
  ///          "1": {
  ///              "type": "HashBuild",
  ///              "fields" : [1]
  ///          },
  ///          "2": {
  ///              "type": "GroupByScan",
  ///              "fields" : [1]
  ///          }
  ///      },
  ///      "edges": [["0", "1"], ["0", "2"], ["1", "2"]]
  ///  }
  static std::shared_ptr<PlanOperation> parse(const Json::Value &v);
  const std::string vname();
  /// creates output result table layout using _field_definitions
  /// and added aggregate functions (aggregate_functions)
  /// _field_definitions member of PlanOperation holds
  /// added (grouping) fields
  storage::atable_ptr_t createResultTableLayout();
  /// adds a given AggregateFunction to group by scan instance SUM or COUNT
  void addFunction(AggregateFun *fun);

private:
  void splitInput();
  void writeGroupResult(storage::atable_ptr_t &resultTab,
                        const std::shared_ptr<storage::pos_list_t> &hit,
                        const size_t row);
  /// Depending on the number of fields to group by choose the appropriate map type
  template<typename HashTableType, typename MapType, typename KeyType>
  void executeGroupBy();

  std::vector<AggregateFun *> _aggregate_functions;

  // global aggregation is used when we cannot guarantee if there is no delta
  // partition containing values. Since the aggregate hash table hashes on
  // valueids instead of values, this can be problematic.
  //
  // Default values is to use the valueID hashing
  bool _globalAggregation = false;
};

}
}

#endif  // SRC_LIB_ACCESS_GROUPBYSCAN_H_
