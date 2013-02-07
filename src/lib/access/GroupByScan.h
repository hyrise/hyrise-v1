// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_GROUPBYSCAN_H_
#define SRC_LIB_ACCESS_GROUPBYSCAN_H_

#include <storage/PointerCalculator.h>

#include "helper/types.h"

#include "PlanOperation.h"
#include "predicates.h"
#include "AggregateFunctions.h"
#include "GroupByScan.h"


/// helper construct to avoid excessive use
/// of switch case in executePlanOperation
/// uses templated type_switch in src/lib/storage/meta_storage.h
/// and calls the the correct templated operator implemented below

namespace hyrise {
namespace storage {
struct write_group_functor {
  typedef void value_type;

  const hyrise::storage::c_atable_ptr_t& input;
  hyrise::storage::atable_ptr_t& target;
  pos_t sourceRow;
  size_t columnNr;
  size_t row;

  write_group_functor(const hyrise::storage::c_atable_ptr_t& t,
                      hyrise::storage::atable_ptr_t& tg,
                      pos_t sourceRow,
                      size_t column,
                      size_t toRow): input(t), target(tg), sourceRow(sourceRow), columnNr(column), row(toRow) {};

  template <typename R>
  void operator()() {
    target->setValue<R>(input->nameOfColumn(columnNr), row, input->getValue<R>(columnNr, sourceRow));
  }
};
}

namespace access {

class GroupByScan : public _PlanOperation {
  std::vector<AggregateFun *> aggregate_functions;

 public:
  static bool is_registered;
  GroupByScan();

  /// adds a given AggregateFunction to group by scan instance SUM or COUNT
  void addFunction(AggregateFun *fun) {
    this->aggregate_functions.push_back(fun);
  }

  virtual ~GroupByScan();

  /// creates output result table layout using _field_definitions
  /// and added aggregate functions (aggregate_functions)
  /// _field_definitions member of _PlanOperation holds
  /// added (grouping) fields
  hyrise::storage::atable_ptr_t createResultTableLayout();

  virtual void executePlanOperation();

  /// Sets up PlanOperation before execution
  /// throws std::runtime_error when no grouping fields are defined but hash input
  /// is available
  void setupPlanOperation();

  /// Parses a GroupByScan
  ///
  /// Reacts to
  /// fields in either integer-list notation or std::string-list notation
  /// functions as a list of {"type": (int|str), "field": (int|str)
  ///
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

  static const std::string name() {
    return "GroupByScan";
  }

  const std::string vname() {
    return "GroupByScan";
  }

 protected:
  void splitInput();
  void writeGroupResult(AbstractTable::SharedTablePtr resultTab, std::shared_ptr<pos_list_t> hit, size_t row);

};

}
}
#endif  // SRC_LIB_ACCESS_GROUPBYSCAN_H_
