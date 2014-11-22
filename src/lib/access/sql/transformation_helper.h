// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_TRANSFORMATIONHELPER_H_
#define SRC_LIB_ACCESS_SQL_TRANSFORMATIONHELPER_H_

#include <vector>
#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/parser/Expr.h"

namespace hyrise {
namespace access {
namespace sql {


#define LOG_META(meta) \
  printf("Columns: "); \
  for (std::string col : meta.fields) printf("%s ", col.c_str()); printf("\n");



/**
 * Combines two vectors into a new single vector
 * Note: since this is a template it has to be explicitely instatiated
 * This happens in the source file. It is instatiated for std::string
 * @param v1	std::vector
 * @param v2	std::vector
 * @return std::vector
 */
template<typename _T>
std::vector<_T> combineVectors(std::vector<_T> v1, std::vector<_T> v2);

/**
 * @param col 	Column that we're trying to find in one of the tables
 * @param t0 	Table 0
 * @param t1 	Table 1
 * @return Returns table index. Returns -2 if it was found in both tables and -1 if it wasn't found at all.
 */
int identifyTableForColumnRef(hsql::Expr* col, TransformationResult t0, TransformationResult t1);

/** 
 * @param func_ref 	Function reference
 * @returns aggregated column name
 */
std::string buildFunctionRefColumnName(hsql::Expr* func_ref);






} // namespace sql
} // namespace access
} // namespace hyrise
#endif