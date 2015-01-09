// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_SQLPREDICATETRANSFORMER_H_
#define SRC_LIB_ACCESS_SQL_SQLPREDICATETRANSFORMER_H_

#include "access/sql/typedef_helper.h"
#include "access/sql/parser/Expr.h"

namespace hyrise {
namespace access {
namespace sql {

class SQLPredicateTransformer {
  typedef enum {
    INT = 0,
    FLOAT = 1,
    STRING = 2
  } ExpressionVType;

 public:
  SQLPredicateTransformer(const TransformationResult& input);
  virtual ~SQLPredicateTransformer() {};

  Json::Value& buildPredicatesFromExpr(hsql::Expr* expr);

 private:
  void parseSimplePredicateIntoJson(hsql::Expr* expr, Json::Value& pred);
  void parsePredicateFieldAndValueIntoJson(hsql::Expr* expr, Json::Value& pred);


  Json::Value _json;
  const TransformationResult& _input;
};






} // namespace sql
} // namespace access
} // namespace hyrise
#endif