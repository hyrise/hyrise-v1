// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_LAYOUTTABLEOP_H_
#define SRC_LIB_ACCESS_LAYOUTTABLEOP_H_

#include "access/PlanOperation.h"

/// Operator to implement re-layouting of one input table
/// into a completely new result table
class LayoutTable : public _PlanOperation {
 public:
  /// Initialize LayoutTable
  /// @param[in] layout string used for layouting the result table
  explicit LayoutTable(const std::string& layout);

  virtual ~LayoutTable();

  void executePlanOperation();

  const std::string vname();

  /// Parses json to create LayoutTable instance
  ///
  ///     { "type": "LayoutTable",
  ///       "layout": "a|b|c\nINTEGER|INTEGER|INTEGER\nC_0|C_1|C_1" }
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
 private:
  const std::string _layout;

  std::shared_ptr<AbstractTable> createEmptyLayoutedTable(const std::string& layout) const;
};

#endif  // SRC_LIB_ACCESS_LAYOUTTABLEOP_H_
