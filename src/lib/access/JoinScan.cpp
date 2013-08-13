// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/JoinScan.h"

#include "access/expressions/expression_types.h"
#include "access/system/QueryParser.h"

#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<JoinScan>("JoinScan");
}

JoinScan::JoinScan(const JoinType::type t) :
                   _join_type(t),
                   _join_condition(nullptr) {
}

JoinScan::~JoinScan() {
  delete _join_condition;
}

void JoinScan::setupPlanOperation() {
  _join_condition->walk(input.getTables());
}

void JoinScan::executePlanOperation() {
  storage::atable_ptr_t left = input.getTable(0)->copy_structure(nullptr, true);
  storage::atable_ptr_t right = input.getTable(1)->copy_structure(nullptr, true);

  storage::pos_t result_row = 0;
  size_t reserved = input.getTable(0)->size() > input.getTable(1)->size() ?
                    input.getTable(0)->size() : input.getTable(1)->size();

  // Reserve memory
  left->reserve(reserved);
  right->reserve(reserved);

  for (storage::pos_t left_row = 0; left_row < input.getTable(0)->size(); left_row++) {
    for (storage::pos_t right_row = 0; right_row < input.getTable(1)->size(); right_row++) {
      if (!_join_condition || (*_join_condition)(left_row, right_row)) {
        left->resize(result_row + 1);
        right->resize(result_row + 1);
        left->copyRowFrom(input.getTable(0), left_row, result_row);
        right->copyRowFrom(input.getTable(1), right_row, result_row++);
      }
    }
  }

  // Create one table
  std::vector<storage::atable_ptr_t > vc;
  vc.push_back(left);
  vc.push_back(right);

  storage::atable_ptr_t result = std::make_shared<storage::MutableVerticalTable>(vc);
  addResult(result);
}

std::shared_ptr<PlanOperation> JoinScan::parse(const Json::Value &v) {
  JoinType::type t = JoinType::type(v["join_type"].asUInt());
  std::shared_ptr<JoinScan> s = std::make_shared<JoinScan>(t);

  for (unsigned i = 0; i < v["predicates"].size(); ++i) {
    Json::Value p = v["predicates"][i];
    if (parseExpressionType(p["type"]) == EXP_EQ) {
      s->addJoinClause<int>(p);
    } else {
      s->addCombiningClause(ExpressionType(p["type"].asUInt()));
    }
  }

  return s;
}

const std::string JoinScan::vname() {
  return "JoinScan";
}

void JoinScan::addCombiningClause(const ExpressionType etype) {
  CompoundJoinExpression *c = new CompoundJoinExpression(etype);
  if (_join_condition == nullptr) {
    _join_condition = c;
  } else {
    _compound_stack.top()->rhs = c;
  }
  _compound_stack.push(c);
}

void JoinScan::addJoinExpression(JoinExpression *expression) {
  if (_join_condition == nullptr) {
    _join_condition = expression;
  } else {
    if (_compound_stack.size() == 0)
      throw std::runtime_error("Compound expression stack is empty");
    CompoundJoinExpression *top =  _compound_stack.top();
    if (top->rhs == nullptr) top->rhs = expression;
    else if (top->lhs == nullptr) top->lhs = expression;
    else throw std::runtime_error("Invalid expression");
    if (top->rhs != nullptr && top->lhs != NULL) _compound_stack.pop();
  }
}

}
}
