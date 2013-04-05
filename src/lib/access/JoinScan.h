// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_JOINSCAN_H_
#define SRC_LIB_ACCESS_JOINSCAN_H_

#include "access/PlanOperation.h"

#include <stack>

#include "access/predicates.h"
#include "helper/types.h"

/// Defines the std::string appended to columns when renaming is necessary
#define RENAMED_COLUMN_APPENDIX_LEFT "_0"
#define RENAMED_COLUMN_APPENDIX_RIGHT "_1"

namespace hyrise {
namespace access {

struct JoinType {
  enum type {
    EQUI,
    INNER,
    OUTER
  };
};

/// A join statement takes two tables as input. For all join types
/// there must be predicates specifying the join condition for the
/// input tables
class JoinScan: public _PlanOperation {
public:
  JoinScan(const JoinType::type t);
  virtual ~JoinScan();

  void setupPlanOperation();
  void executePlanOperation();
  /// { type: "JoinScan", jtype: "EQUI", predicates: [{type: 0}, {type: 3, in: 0, f:0}, {type: "3"] }
  static std::shared_ptr<_PlanOperation> parse(const Json::Value &v);
  const std::string vname();
  template<typename T>
  void addJoinClause(const size_t input_left,
                     const storage::field_t field_left,
                     const size_t input_right,
                     const storage::field_t field_right);
  template<typename T>
  void addJoinClause(const Json::Value &value);
  void addCombiningClause(const ExpressionType t);

private:
  void addJoinExpression(JoinExpression *);
  JoinType::type _join_type;
  JoinExpression *_join_condition;
  std::stack<CompoundJoinExpression *> _compound_stack;
};

template<typename T>
void JoinScan::addJoinClause(const size_t input_left,
                             const storage::field_t field_left,
                             const size_t input_right,
                             const storage::field_t field_right) {
  auto expr1 = new EqualsJoinExpression<T>(input_left,
                                           field_left,
                                           input_right,
                                           field_right);
  addJoinExpression(expr1);
}

template<typename T>
void JoinScan::addJoinClause(const Json::Value &value) {
  EqualsJoinExpression<T> *expr1 = EqualsJoinExpression<T>::parse(value);
  addJoinExpression(expr1);
}

}
}

#endif  // SRC_LIB_ACCESS_JOINSCAN_H_
