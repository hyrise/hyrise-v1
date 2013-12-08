// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "expression_types.h"

#include <stdexcept>

namespace hyrise {
namespace access {

expression_map_t getExpressionMap() {
  expression_map_t d;
  d["AND"] = AND;
  d["OR"] = OR;
  d["NOT"] = NOT;
  d["EQ"] =  EXP_EQ;
  return d;
}

predicate_map_t getPredicateMap() {
  predicate_map_t d;
  d["EQ"] = PredicateType::EqualsExpression;
  d["EQ_V"] = PredicateType::EqualsExpressionValue;
  d["LT"] = PredicateType::LessThanExpression;
  d["GT"] = PredicateType::GreaterThanExpression;
  d["LT_V"] = PredicateType::LessThanExpressionValue;
  d["GT_V"] = PredicateType::GreaterThanExpressionValue;
  d["LTE_V"] = PredicateType::LessThanEqualsExpressionValue;
  d["GTE_V"] = PredicateType::GreaterThanEqualsExpressionValue;
  d["EQ_R"] = PredicateType::EqualsExpressionRaw;
  d["LT_R"] = PredicateType::LessThanExpressionRaw;
  d["GT_R"] = PredicateType::GreaterThanExpressionRaw;
  d["BETWEEN"] = PredicateType::BetweenExpression;
  d["COMPOUND"] = PredicateType::CompoundExpression;
  d["NEG"] = PredicateType::NegateExpression;
  d["AND"] = PredicateType::AND;
  d["OR"] = PredicateType::OR;
  d["NOT"] = PredicateType::NOT;
  d["LIKE"] = PredicateType::LikeExpression;
  d["IN"] = PredicateType::InExpression;
  return d;
}

PredicateType::type parsePredicateType(const Json::Value &value) {
  if (value.isString()) return getPredicateMap()[value.asString()];
  else if (value.isNumeric()) return (PredicateType::type) value.asInt();
  else throw std::runtime_error("Predicate '" + value.asString() + "' could not be parsed");
}

ExpressionType parseExpressionType(const Json::Value &value) {
  if (value.isString()) return getExpressionMap()[value.asString()];
  else if (value.isNumeric()) return (ExpressionType) value.asInt();
  else throw std::runtime_error("Expression '" + value.asString() + "' could not be parsed");
}

} } // namespace hyrise::access

