// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "predicate_helper.h"
#include "access/expressions/expression_types.h"

#include <log4cxx/logger.h>

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

namespace {
  log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access"));
}


struct HyriseVTypes {
  typedef enum {
    INT = 0,
    FLOAT = 1,
    STRING = 2
  } Type;
};


void parsePredicateFieldAndValueIntoJson(Expr* expr, Json::Value& pred) {
  // We assume that one of the two expressions has to be a columnref
  // while the other is a literal.
  // This is a requirement from the expression engine in hyrise.
  bool first_expr_is_column_ref = (expr->expr->type == kExprColumnRef);
  Expr* column_expr = (first_expr_is_column_ref) ? expr->expr : expr->expr2;
  Expr* value_expr = (!first_expr_is_column_ref) ? expr->expr : expr->expr2;

  pred["f"] = column_expr->name;

  switch (value_expr->type) {
    case kExprLiteralInt:
      pred["vtype"] = HyriseVTypes::INT;
      pred["value"] = value_expr->ival;
      break;
    case kExprLiteralFloat:
      pred["vtype"] = HyriseVTypes::FLOAT;
      pred["value"] = value_expr->fval;
      break;
    case kExprLiteralString:
      pred["vtype"] = HyriseVTypes::STRING;
      pred["value"] = value_expr->name;
      break;
    default:
      throw std::runtime_error("Error when transforming SQL: Predicate expressions not supported");
  }
}


void parseSimplePredicateIntoJson(Expr* expr, Json::Value& pred) {
  pred["in"] = 0;
  parsePredicateFieldAndValueIntoJson(expr, pred);

  switch (expr->op_char) {
    case '=': pred["type"] = PredicateType::EqualsExpression; break;
    case '<': pred["type"] = PredicateType::LessThanExpression; break;
    case '>': pred["type"] = PredicateType::GreaterThanExpression; break;
    default:
      throw std::runtime_error("Error when transforming SQL: Currently not supported operator type in where clause");
  }
}


void buildPredicatesFromSQLExpr(Expr* expr, Json::Value& out_predicates) {
  if (expr->type != kExprOperator) {
    // Hyrise only allows Operator predicates
    throw std::runtime_error("Error when transforming SQL: Expression type not supported in where clause");
  }

  Json::Value pred;

  switch (expr->op_type) {
    case Expr::SIMPLE_OP: {
      parseSimplePredicateIntoJson(expr, pred);
      out_predicates.append(pred);
      break;
    }

    case Expr::LESS_EQ:
      pred["in"] = 0;
      pred["type"] = PredicateType::LessThanEqualsExpressionValue;
      parsePredicateFieldAndValueIntoJson(expr, pred);
      out_predicates.append(pred);
      break;

    case Expr::GREATER_EQ:
      pred["in"] = 0;
      pred["type"] = PredicateType::GreaterThanEqualsExpressionValue;
      parsePredicateFieldAndValueIntoJson(expr, pred);
      out_predicates.append(pred);
      break;

    case Expr::NOT_EQUALS: {
      Json::Value not_pred;
      not_pred["type"] = PredicateType::NOT;
      out_predicates.append(not_pred);

      expr->op_char = '=';
      parseSimplePredicateIntoJson(expr, pred);
      out_predicates.append(pred);
      break;
    }

    case Expr::LIKE:
      pred["in"] = 0;
      pred["type"] = PredicateType::LikeExpression;
      parsePredicateFieldAndValueIntoJson(expr, pred);
      out_predicates.append(pred);
      break;

    case Expr::NOT_LIKE: {
      Json::Value not_pred;
      not_pred["type"] = PredicateType::NOT;
      out_predicates.append(not_pred);

      Json::Value like_pred;
      pred["in"] = 0;
      pred["type"] = PredicateType::LikeExpression;
      parsePredicateFieldAndValueIntoJson(expr, pred);
      out_predicates.append(pred);
      break;
    }

    /**
     * Hyrise compound expressions
     */
    case Expr::AND:
      pred["type"] = PredicateType::AND;
      out_predicates.append(pred);
      buildPredicatesFromSQLExpr(expr->expr, out_predicates);
      buildPredicatesFromSQLExpr(expr->expr2, out_predicates);
      break;

    case Expr::OR:
      pred["type"] = PredicateType::OR;
      out_predicates.append(pred);
      buildPredicatesFromSQLExpr(expr->expr, out_predicates);
      buildPredicatesFromSQLExpr(expr->expr2, out_predicates);
      break;

    case Expr::NOT:
      pred["type"] = PredicateType::NOT;
      out_predicates.append(pred);
      buildPredicatesFromSQLExpr(expr->expr, out_predicates);
      break;

    default:
      throw std::runtime_error("Error when transforming SQL: Currently not supported operator type in where clause");
  }

}


} // namespace sql
} // namespace access
} // namespace hyrise