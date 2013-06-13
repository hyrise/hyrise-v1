// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPRESSION_TYPES_H_
#define SRC_LIB_ACCESS_EXPRESSION_TYPES_H_

#include <map>
#include <string>

#include <json.h>

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
    GreaterThanExpressionRaw = 15
  } type;
};

typedef std::map<std::string, PredicateType::type> predicate_map_t;
typedef std::map<std::string, ExpressionType> expression_map_t;

PredicateType::type parsePredicateType(const Json::Value &value);
ExpressionType parseExpressionType(const Json::Value &value);


#endif  // SRC_LIB_ACCESS_EXPRESSION_TYPES_H_
