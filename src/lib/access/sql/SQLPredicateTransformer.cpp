#include "access/sql/SQLPredicateTransformer.h"
#include "access/sql/SQLStatementTransformer.h"
#include "access/expressions/expression_types.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

SQLPredicateTransformer::SQLPredicateTransformer(const TransformationResult& input) :
	_input(input) {

}


Json::Value& SQLPredicateTransformer::buildPredicatesFromExpr(Expr* expr) {
  if (expr->type != kExprOperator) {
    // Hyrise only allows Operator predicates
    throw std::runtime_error("Error when transforming SQL: Expression type not supported in where clause");
  }

  Json::Value pred;

  switch (expr->op_type) {
    case Expr::SIMPLE_OP: {
      parseSimplePredicateIntoJson(expr, pred);
      _json.append(pred);
      break;
    }

    case Expr::LESS_EQ:
      pred["in"] = 0;
      pred["type"] = PredicateType::LessThanEqualsExpressionValue;
      parsePredicateFieldAndValueIntoJson(expr, pred);
      _json.append(pred);
      break;

    case Expr::GREATER_EQ:
      pred["in"] = 0;
      pred["type"] = PredicateType::GreaterThanEqualsExpressionValue;
      parsePredicateFieldAndValueIntoJson(expr, pred);
      _json.append(pred);
      break;

    case Expr::NOT_EQUALS: {
      Json::Value not_pred;
      not_pred["type"] = PredicateType::NOT;
      _json.append(not_pred);

      expr->op_char = '=';
      parseSimplePredicateIntoJson(expr, pred);
      _json.append(pred);
      break;
    }

    case Expr::LIKE:
      pred["in"] = 0;
      pred["type"] = PredicateType::LikeExpression;
      parsePredicateFieldAndValueIntoJson(expr, pred);
      _json.append(pred);
      break;

    case Expr::NOT_LIKE: {
      Json::Value not_pred;
      not_pred["type"] = PredicateType::NOT;
      _json.append(not_pred);

      Json::Value like_pred;
      pred["in"] = 0;
      pred["type"] = PredicateType::LikeExpression;
      parsePredicateFieldAndValueIntoJson(expr, pred);
      _json.append(pred);
      break;
    }

    /**
     * Hyrise compound expressions
     */
    case Expr::AND:
      pred["type"] = PredicateType::AND;
      _json.append(pred);
      buildPredicatesFromExpr(expr->expr);
      buildPredicatesFromExpr(expr->expr2);
      break;

    case Expr::OR:
      pred["type"] = PredicateType::OR;
      _json.append(pred);
      buildPredicatesFromExpr(expr->expr);
      buildPredicatesFromExpr(expr->expr2);
      break;

    case Expr::NOT:
      pred["type"] = PredicateType::NOT;
      _json.append(pred);
      buildPredicatesFromExpr(expr->expr);
      break;

    default:
      throw std::runtime_error("Error when transforming SQL: Currently not supported operator type in where clause");
  }

  return _json;
}


void SQLPredicateTransformer::parseSimplePredicateIntoJson(Expr* expr, Json::Value& pred) {
  pred["in"] = 0;
  parsePredicateFieldAndValueIntoJson(expr, pred);

  switch (expr->op_char) {
    case '=': pred["type"] = PredicateType::EqualsExpressionValue; break;
    case '<': pred["type"] = PredicateType::LessThanExpressionValue; break;
    case '>': pred["type"] = PredicateType::GreaterThanExpressionValue; break;
    default:
      throw std::runtime_error("Error when transforming SQL: Currently not supported operator type in where clause");
  }
}



void SQLPredicateTransformer::parsePredicateFieldAndValueIntoJson(Expr* expr, Json::Value& pred) {
  // We assume that one of the two expressions has to be a columnref
  // while the other is a literal.
  // This is a requirement from the expression engine in hyrise.
  bool first_expr_is_column_ref = (expr->expr->type == kExprColumnRef);
  Expr* column_expr = (first_expr_is_column_ref) ? expr->expr : expr->expr2;
  Expr* value_expr = (!first_expr_is_column_ref) ? expr->expr : expr->expr2;

  int id = -1;
  if (column_expr->table == nullptr) id = _input.getFieldID(column_expr->name, "");
  else id = _input.getFieldID(column_expr->name, column_expr->table);

  if (id < 0) SQLStatementTransformer::throwError("Column not found in table", column_expr->name);

  pred["f"] = id;

  switch (value_expr->type) {
    case kExprLiteralInt:
      pred["vtype"] = ExpressionVType::INT;
      pred["value"] = value_expr->ival;
      break;
    case kExprLiteralFloat:
      pred["vtype"] = ExpressionVType::FLOAT;
      pred["value"] = value_expr->fval;
      break;
    case kExprLiteralString:
      pred["vtype"] = ExpressionVType::STRING;
      pred["value"] = value_expr->name;
      break;
    default:
      throw std::runtime_error("Error when transforming SQL: Predicate expressions not supported");
  }
}


} // namespace sql
} // namespace access
} // namespace hyrise