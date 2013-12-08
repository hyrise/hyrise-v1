// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <map>
#include <string>

#include <json.h>

namespace hyrise {
namespace access {

enum ExpressionType { AND = 0, OR = 1, NOT = 2, EXP_EQ = 3 };

struct PredicateType {
  typedef enum {
    EqualsExpression = 0,
    LessThanExpression = 1,
    GreaterThanExpression = 2,
    BetweenExpression = 3,
    CompoundExpression = 4,
    NegateExpression = 5,
    AND = 6,
    OR = 7,
    NOT = 8,
    EqualsExpressionRaw = 13,
    LessThanExpressionRaw = 14,
    GreaterThanExpressionRaw = 15,
    LikeExpression = 16,
    InExpression = 17,

    EqualsExpressionValue = 20,
    GreaterThanExpressionValue = 21,
    LessThanExpressionValue = 22,
    GreaterThanEqualsExpressionValue = 23,
    LessThanEqualsExpressionValue = 24
  } type;
};

typedef std::map<std::string, PredicateType::type> predicate_map_t;
typedef std::map<std::string, ExpressionType> expression_map_t;

PredicateType::type parsePredicateType(const Json::Value &value);
ExpressionType parseExpressionType(const Json::Value &value);

} } // namespace hyrise::access

