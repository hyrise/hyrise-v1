// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_LAYOUTTABLEOP_H_
#define SRC_LIB_ACCESS_LAYOUTTABLEOP_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// Operator to implement re-layouting of one input table
/// into a completely new result table
class LayoutTable : public _PlanOperation {
public:
  explicit LayoutTable(const std::string &layout);
  virtual ~LayoutTable();

  void executePlanOperation();
  /// { "type": "LayoutTable",
  ///   "layout": "a|b|c\nINTEGER|INTEGER|INTEGER\nC_0|C_1|C_1" }
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();

private:
  storage::atable_ptr_t createEmptyLayoutedTable(const std::string &layout) const;
  const std::string _layout;
};

}
}

#endif  // SRC_LIB_ACCESS_LAYOUTTABLEOP_H_
