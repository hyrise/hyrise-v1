// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_GROUPBYSCAN_H_
#define SRC_LIB_ACCESS_GROUPBYSCAN_H_

#include "access/PlanOperation.h"
#include "access/AggregateFunctions.h"
#include "helper/types.h"

namespace hyrise {
namespace storage {

/// helper construct to avoid excessive use
/// of switch case in executePlanOperation
/// uses templated type_switch in src/lib/storage/meta_storage.h
/// and calls the the correct templated operator implemented below
struct write_group_functor {
public:
  typedef void value_type;
  write_group_functor(const c_atable_ptr_t& t,
                      atable_ptr_t& tg,
                      pos_t sourceRow,
                      field_t column,
                      pos_t toRow);

  template <typename R>
  void operator()();
private:
  const c_atable_ptr_t &_input;
  atable_ptr_t &_target;
  pos_t _sourceRow;
  field_t _columnNr;
  pos_t _row;
};

}

namespace access {

class GroupByScan : public _PlanOperation {
 public:
  GroupByScan();
  virtual ~GroupByScan();

  /// creates output result table layout using _field_definitions
  /// and added aggregate functions (aggregate_functions)
  /// _field_definitions member of _PlanOperation holds
  /// added (grouping) fields
  storage::atable_ptr_t createResultTableLayout();
  void setupPlanOperation();
  virtual void executePlanOperation();
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
  static std::shared_ptr<_PlanOperation> parse(Json::Value &v);
  const std::string vname();
  /// adds a given AggregateFunction to group by scan instance SUM or COUNT
  void addFunction(AggregateFun *fun);

 protected:
  void splitInput();
  void writeGroupResult(storage::atable_ptr_t resultTab, std::shared_ptr<storage::pos_list_t> hit, size_t row);

private:
  std::vector<AggregateFun *> _aggregate_functions;
};

}
}
#endif  // SRC_LIB_ACCESS_GROUPBYSCAN_H_
