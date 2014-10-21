// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_ACCESS_SQL_PREDICATEHELPER_H_
#define SRC_LIB_ACCESS_SQL_PREDICATEHELPER_H_

#include "access/sql/parser/Expr.h"
#include <json.h>

namespace hyrise {
namespace access {
namespace sql {

void parsePredicateFieldAndValueIntoJson(hsql::Expr* expr, Json::Value& pred);

void parseSimplePredicateIntoJson(hsql::Expr* expr, Json::Value& pred);

void buildPredicatesFromSQLExpr(hsql::Expr* expr, Json::Value& out_predicates);


} // namespace sql
} // namespace access
} // namespace hyrise


#endif