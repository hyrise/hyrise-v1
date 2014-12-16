// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/sql/SQLSelectTransformer.h"
#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/transformation_helper.h"

#include "access/HashBuild.h"
#include "access/HashJoinProbe.h"
#include "access/GroupByScan.h"
#include "access/SortScan.h"
#include "access/UnionScan.h"
#include "access/ProjectionScan.h"
#include "access/Distinct.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

SQLSelectTransformer::SQLSelectTransformer(SQLStatementTransformer& server) :
  _server(server),
  _builder(server.getTaskListBuilder()) {}

SQLSelectTransformer::~SQLSelectTransformer() {}



TransformationResult SQLSelectTransformer::transformSelectStatement(SelectStatement* stmt) {
  TransformationResult meta = {};
  // SQL Order of Operations: http://www.bennadel.com/blog/70-sql-query-order-of-operations.htm
  // 1. FROM clause
  // 2. WHERE clause
  // 3. GROUP BY clause
  // 4. HAVING clause
  // 5. SELECT clause
  // 6. ORDER BY clause

  TransformationResult table_res = _server.transformTableRef(stmt->from_table);
  meta = table_res;

  // WHERE clause  
  if (stmt->where_clause != nullptr) {
    auto scan = _server.addFilterOpFromExpr(stmt->where_clause, meta);
  }

  // GROUP BY clause
  if (stmt->group_by != nullptr) {
    TransformationResult group_res = transformGroupByClause(stmt, meta);
    meta.append(group_res);

    // Having
    if (stmt->group_by->having != nullptr)
      auto scan = _server.addFilterOpFromExpr(stmt->group_by->having, meta);
  }


  // SELECT clause
  if (stmt->select_distinct) transformDistinctSelection(stmt, meta);

  // If there is only SELECT * specified we don't have to do a projection
  if (stmt->select_list->at(0)->type != kExprStar || 
      stmt->select_list->size() > 1) {
    transformSelectionList(stmt, meta);
  }

  // ORDER BY clause
  if (stmt->order != nullptr) {
    if (!stmt->order->expr->isType(kExprColumnRef)) {
      _server.throwError("Order By expression has to be a column reference");
    }

    auto scan = std::make_shared<SortScan>();//_server.addNewPlanOp<SortScan>("SortScan", meta);
    scan->setSortField(meta.getFieldID(stmt->order->expr->name));
    scan->setAsc(stmt->order->type == kOrderAsc);
    _builder.addPlanOp(scan, "SortScan", meta);
  }

  // LIMIT clause
  if (stmt->limit != nullptr) {
    // Projection Scan is the only op I could find that supports limit
    // Problem: Offset not supported by operator
    if (stmt->limit->offset != kNoOffset) _server.throwError("Offset not supported yet");
    
    auto scan = std::make_shared<ProjectionScan>();
    scan->addField("*");
    scan->setLimit(stmt->limit->limit);
    _builder.addPlanOp(scan, "Limit-ProjectionScan", meta);
  }

  // UNION
  // TODO: Problem: UnionScan can only work on the same table and only if positions have been produced
  if (stmt->union_select != nullptr) {
    TransformationResult sub_select = transformSelectStatement(stmt->union_select);
    auto scan = std::make_shared<UnionScan>();
    _builder.addPlanOp(scan, "UnionScan", meta);
    scan->addDependency(sub_select.last_task);
  }

  return meta;
}




/**
 * Transform the group by clause of a SelectStatement.
 *
 * @param[in]  stmt       SelectStatement thats selections list will be transformed
 * @return  Information about the result of the transformation
 */
TransformationResult SQLSelectTransformer::transformGroupByClause(SelectStatement* stmt, const TransformationResult& input) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
  meta.name = input.name;
  List<Expr*>* groups = stmt->group_by->columns;

  // Build Hash Table
  std::shared_ptr<HashBuild> hash = std::make_shared<HashBuild>();
  hash->setKey("groupby");
  for (Expr* expr : groups->vector()) {
    if (expr->type == kExprColumnRef) hash->addField(expr->name);
    else _server.throwError("Unexpected Expression Type in Group By");
  }

  // Build GroupByScan
  std::shared_ptr<GroupByScan> scan = std::make_shared<GroupByScan>();
  for (Expr* expr : groups->vector()) {
    if (expr->type == kExprColumnRef) {
      int id = input.getFieldID(expr);
      scan->addField(id);
      meta.addField(expr->name, input.keys[id], input.data_types[id]); // Add to result schema
    } else _server.throwError("Unexpected Expression Type in Group By");
  }
  // aggregations
  for (Expr* expr : stmt->select_list->vector()) {
    if (expr->type == kExprFunctionRef) {
      std::string func_name = expr->name;
      std::string column_name = (expr->alias == nullptr) ? buildFunctionRefColumnName(expr) : expr->alias;
      Expr* subexpr = expr->expr;

      if (subexpr->type == kExprColumnRef) {
        Json::Value json;
        json["type"] = stringToUppercase(func_name);
        json["field"] = subexpr->name;
        json["as"] = column_name;
        json["distinct"] = expr->distinct;
        scan->addFunction(parseAggregateFunction(json));
        meta.addField(column_name, "", TableInfo::kUnknown); // Add to result schema
      } else {
        _server.throwError("Unexpected Expression Type in Aggregation");
      }
    }
  }

  // Link tasks and build result
  hash->addDependency(input.last_task);
  _builder.addPlanOp(hash, "HashBuild");

  scan->addDependency(input.last_task);
  scan->addDependency(hash);
  _builder.addPlanOp(scan, "GroupByScan");

  meta.addTask(hash);
  meta.addTask(scan);
  return meta;
}



/**
 * Transform the selection list of a SelectStatement into a ProjectionScan task.
 *
 * @param[in]  stmt       SelectStatement thats selections list will be transformed
 * @param[in]  input       Information about the source for the projection
 * @return  Information about the result of the transformation
 */
void SQLSelectTransformer::transformSelectionList(SelectStatement* stmt, TransformationResult& meta) {
  plan_op_t scan = std::make_shared<ProjectionScan>();

  TableInfo info = addFieldsFromExprListToScan(scan, stmt->select_list, meta);

  // update table info
  meta.useColumnInfoFrom(info);

  _builder.addPlanOp(scan, "ProjectionScan", meta);
}

void SQLSelectTransformer::transformDistinctSelection(hsql::SelectStatement* stmt, TransformationResult& meta) {
  plan_op_t scan = std::make_shared<Distinct>();

  TableInfo info = addFieldsFromExprListToScan(scan, stmt->select_list, meta);
  if (info.numColumns() > 1) _server.throwError("SELECT DISTINCT can only handle single columns at the moment.");

  _builder.addPlanOp(scan, "DistinctScan", meta);
}

TableInfo SQLSelectTransformer::addFieldsFromExprListToScan(plan_op_t scan, List<Expr*>* list, const TableInfo& input) {
  // If we are not selecting everything, we have to perform a projection scan
  // Problem: can't select literals with this operator
  TableInfo out_info;

  for (Expr* expr : list->vector()) {
    switch (expr->type) {
      case kExprColumnRef: {
        int id = input.getFieldID(expr);

        if (id < 0) _server.throwError("Column could not be found!", expr->name);
        scan->addField(id);
        out_info.addFieldFromInfo(input, id);

        break;
      }
      case kExprStar:
        // This can only work if there is meta information available about the table
        for (size_t i = 0; i < input.numColumns(); ++i) {
          scan->addField(i);
          out_info.addFieldFromInfo(input, i);
        }
        break;
      case kExprFunctionRef: {
        // Assumption: We had a group by previously
        std::string name = (expr->alias == nullptr) ? buildFunctionRefColumnName(expr) : expr->alias;
        int id = input.getFieldID(name, "");
        
        if (id < 0) _server.throwError("Column could not be found!", expr->name);
        scan->addField(id);
        out_info.addField(name, "", TableInfo::kUnknown);
        break;
      }
      default: 
        _server.throwError("Expression type not supported in SELECT clause");
        break;
    }
  }
  return out_info;
}

} // namespace sql
} // namespace access
} // namespace hyrise