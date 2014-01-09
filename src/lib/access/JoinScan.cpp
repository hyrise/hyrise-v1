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
  if (_join_type != JoinType::EQUI) {
    throw std::runtime_error("Currently only JoinType::EQUI supported by JoinScan");
  }

  if (!_join_condition) {
    throw std::runtime_error("JoinScan needs a defined join condition");
  }

  auto left_source = input.getTable(0),
      right_source = input.getTable(1);

  storage::atable_ptr_t left_target = left_source->copy_structure(nullptr, true);
  storage::atable_ptr_t right_target = right_source->copy_structure(nullptr, true);

  storage::pos_t result_row = 0;
  for (storage::pos_t left_row = 0, left_size = left_source->size(); left_row < left_size; ++left_row) {
    for (storage::pos_t right_row = 0, right_size = right_source->size(); right_row < right_size; ++right_row) {
      if ((*_join_condition)(left_row, right_row)) {
        left_target->resize(result_row + 1);
        right_target->resize(result_row + 1);
        left_target->copyRowFrom(left_source, left_row, result_row);
        right_target->copyRowFrom(right_source, right_row, result_row);
        result_row++;
      }
    }
  }

  addResult(std::make_shared<storage::MutableVerticalTable>(
      std::vector<storage::atable_ptr_t> {left_target, right_target}));
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
    if (top->rhs != nullptr && top->lhs != nullptr) _compound_stack.pop();
  }
}

}
}
