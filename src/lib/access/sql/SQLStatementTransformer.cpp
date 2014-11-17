// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/predicate_helper.h"
#include "access/sql/parser/sqlhelper.h"

#include "io/StorageManager.h"

#include "access/expressions/pred_buildExpression.h"

// Operators
#include "access/storage/TableLoad.h"
#include "access/storage/GetTable.h"
#include "access/HashBuild.h"
#include "access/SimpleTableScan.h"
#include "access/UnionScan.h"
#include "access/JoinScan.h"
#include "access/ProjectionScan.h"
#include "access/GroupByScan.h"
#include "access/NoOp.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

#define LOG_META(meta) for (int i = 0; i < meta.num_columns; ++i) printf("%s ", meta.column_names[i].c_str()); printf("\n");

namespace { log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access")); }

/** 
 * buildFunctionRefColumnName
 *
 *
 */
std::string buildFunctionRefColumnName(Expr* func_ref) {
  std::string column_name = std::string(func_ref->name) + "(" + std::string(func_ref->expr->name) + ")";
  return column_name;
}



/** 
 * transformStatement
 *
 *
 */
TransformationResult SQLStatementTransformer::transformStatement(Statement* stmt, task_list_t& task_list) {
  switch (stmt->type) {
    case kStmtSelect: return transformSelectStatement((SelectStatement*)stmt, task_list);
    case kStmtCreate: return transformCreateStatement((CreateStatement*)stmt, task_list);
    default: throwError("Unsupported statement type!\n");
  }
  return ALLOC_TRANSFORMATIONRESULT();
}

/** 
 * transformCreateStatement
 *
 *
 */
TransformationResult SQLStatementTransformer::transformCreateStatement(CreateStatement* stmt, task_list_t& task_list) {
  if (io::StorageManager::getInstance()->exists(stmt->table_name)) {
    // TODO: This should be handled by the plan operator that is used
    throwError("Table already exists");
  }

  std::shared_ptr<TableLoad> table_load = std::make_shared<TableLoad>();
  table_load->setTableName(std::string(stmt->table_name));
  table_load->setFileName(std::string(stmt->file_path));
  appendPlanOp(table_load, "TableLoad", task_list);

  std::shared_ptr<NoOp> no_op = std::make_shared<NoOp>();
  no_op->addDependency(table_load);
  appendPlanOp(no_op, "NoOp", task_list);
  
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
  meta.first_task = table_load;
  meta.last_task = no_op;
  return meta;
}

/** 
 * transformSelectStatement
 *
 *
 */
TransformationResult SQLStatementTransformer::transformSelectStatement(SelectStatement* stmt, task_list_t& task_list) {
  printSelectStatementInfo(stmt, 0);
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();

  // SQL Order of Operations: http://www.bennadel.com/blog/70-sql-query-order-of-operations.htm
  // 1. FROM clause
  // 2. WHERE clause
  // 3. GROUP BY clause
  // 4. HAVING clause (TODO)
  // 5. SELECT clause
  // 6. ORDER BY clause (TODO)


  // FROM clause
  TransformationResult table_res = transformTableRef(stmt->from_table, task_list);
  meta = table_res;

  // WHERE clause  
  if (stmt->where_clause != nullptr) {
    // If we have a where clause specified, we need to build a simple table scan over the result
    // Problem: Expression engine only allows comparisons like this: COLUMN = literal
    // we can't have arithmetic sub expressions or expressions consisting of multiple columns
    // TODO: Supply the data types to the expression builder from meta information
    Json::Value predicates;
    buildPredicatesFromSQLExpr(stmt->where_clause, predicates);

    std::shared_ptr<SimpleTableScan> scan = std::make_shared<SimpleTableScan>();
    scan->setPredicate(hyrise::access::buildExpression(predicates));
    scan->addDependency(meta.last_task);
    appendPlanOp(scan, "SimpleTableScan", task_list);
    meta.last_task = scan;
  } 

  // GROUP BY clause
  if (stmt->group_by != nullptr) {
    TransformationResult group_res = transformGroupByClause(stmt, task_list);
    meta.num_columns = group_res.num_columns;
    meta.column_names = group_res.column_names;
    meta.data_types = group_res.data_types;
    meta.last_task = group_res.last_task;
  }

  // SELECT clause
  // If there is only SELECT * specified we usually don't have to do a projection
  // but since a union also requires a produced position list, we remove the condition here for now
  // to make sure that positions have been produced
  // TODO: fix
  if (stmt->select_list->at(0)->type != kExprStar || stmt->select_list->size() > 1) {
  // if (true) {
    TransformationResult res = transformSelectionList(stmt, meta, task_list);
    meta.num_columns = res.num_columns;
    meta.column_names = res.column_names;
    meta.data_types = res.data_types;
    meta.last_task = res.last_task;
  }

  // ORDER BY clause
  if (stmt->order != nullptr) {
    throwError("Order By not supported yet");
  }

  // LIMIT clause
  if (stmt->limit != nullptr) {
    // Projection Scan is the only op I found that supports limit
    // Problem: Offset not supported by operator
    if (stmt->limit->offset != kNoOffset) throwError("Offset not supported yet");
    
    std::shared_ptr<ProjectionScan> scan = std::make_shared<ProjectionScan>();
    for (std::string column : meta.column_names) scan->addField(column);
    scan->setLimit(stmt->limit->limit);
    scan->addDependency(meta.last_task);
    appendPlanOp(scan, "LimitScan", task_list);
    meta.last_task = scan;
  }


  // UNION
  // Problem: UnionScan can only work on the same table, if positions have been produced
  if (stmt->union_select != nullptr) {
    TransformationResult t_res = transformSelectStatement(stmt->union_select, task_list);

    std::shared_ptr<UnionScan> scan = std::make_shared<UnionScan>();
    scan->addDependency(meta.last_task);
    scan->addDependency(t_res.last_task);
    appendPlanOp(scan, "UnionScan", task_list);
    meta.last_task = scan;
  }

  // Expected result
  // LOG_META(meta);

  return meta;
}


/**
 * Transform the group by clause of a SelectStatement.
 *
 * @param[in]  stmt       SelectStatement thats selections list will be transformed
 * @param[out] task_list  Reference to total task list
 * @return  Information about the result of the transformation
 */
TransformationResult SQLStatementTransformer::transformGroupByClause(SelectStatement* stmt, task_list_t& task_list) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
  task_t input_task = task_list.back();
  List<Expr*>* groups = stmt->group_by;

  // Build Hash Table
  std::shared_ptr<HashBuild> hash = std::make_shared<HashBuild>();
  hash->setKey("groupby");
  for (Expr* expr : groups->vector()) {
    if (expr->type == kExprColumnRef) hash->addField(expr->name);
    else throwError("Unexpected Expression Type in Group By");
  }

  // Build GroupByScan
  std::shared_ptr<GroupByScan> scan = std::make_shared<GroupByScan>();
  for (Expr* expr : groups->vector()) {
    if (expr->type == kExprColumnRef) {
      scan->addField(expr->name);
      meta.addField(expr->name); // Add to result schema
    } else throwError("Unexpected Expression Type in Group By");
  }
  // aggregations
  for (Expr* expr : stmt->select_list->vector()) {
    if (expr->type == kExprFunctionRef) {
      std::string func_name = expr->name;
      std::string column_name = (expr->alias == NULL) ? buildFunctionRefColumnName(expr) : expr->alias;
      Expr* subexpr = expr->expr;

      if (subexpr->type == kExprColumnRef) {
        Json::Value json;
        json["type"] = func_name; // TODO: convert to uppercase
        json["field"] = subexpr->name;
        json["as"] = column_name;
        scan->addFunction(parseAggregateFunction(json));
        meta.addField(column_name); // Add to result schema
      } else {
        throwError("Unexpected Expression Type in Aggregation");
      }
    }
  }

  // Link tasks and build result
  hash->addDependency(input_task);
  appendPlanOp(hash, "HashBuild", task_list);

  scan->addDependency(input_task);
  scan->addDependency(hash);
  appendPlanOp(scan, "GroupByScan", task_list);

  meta.first_task = hash;
  meta.last_task = scan;

  return meta;
}


/**
 * Transform the selection list of a SelectStatement into a ProjectionScan task.
 *
 * @param[in]  stmt       SelectStatement thats selections list will be transformed
 * @param[in]  info       Information about the source for the projection
 * @param[out] task_list  Reference to total task list
 * @return  Information about the result of the transformation
 */
TransformationResult SQLStatementTransformer::transformSelectionList(hsql::SelectStatement* stmt, TransformationResult info, task_list_t& task_list) {
  TransformationResult res = ALLOC_TRANSFORMATIONRESULT();
  // If we are not selecting everything, we have to perform a projection scan
  // Problem: can't select literals with this operator

  std::shared_ptr<ProjectionScan> scan = std::make_shared<ProjectionScan>();

  for (Expr* expr : stmt->select_list->vector()) {
    switch (expr->type) {
      case kExprColumnRef:
        scan->addField(expr->name);
        res.addField(expr->name);
        break;
      case kExprStar:
        // This can only work if there is meta information available about the table
        for (int i = 0; i < info.num_columns; ++i) {
          scan->addField(info.column_names[i]);
          res.addField(info.column_names[i]);
        }
        break;
      case kExprFunctionRef: {
        // Assumption: We had a group by previously
        std::string name = (expr->alias == NULL) ? buildFunctionRefColumnName(expr) : expr->alias;
        scan->addField(name);
        res.addField(name);
        break;
      }
      default: break;
    }
  }

  scan->addDependency(info.last_task);
  appendPlanOp(scan, "ProjectionScan", task_list);
  res.last_task = scan;
  return res;
}

/**
 * Transform the table ref into the equivalent hyrise taskts
 *
 * @param[in]  table_ref  TableRef that will be transformed
 * @param[out] task_list  Reference to total task list
 * @return  Information about the result of the transformation
 */
TransformationResult SQLStatementTransformer::transformTableRef(TableRef* table_ref, task_list_t& task_list) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();

  switch (table_ref->type) {
    case kTableName: {
      std::shared_ptr<GetTable> get_table = std::make_shared<GetTable>(table_ref->name);
      appendPlanOp(get_table, "GetTable", task_list);
      meta.first_task = get_table;
      meta.last_task = get_table;

      // Get table definition, if it is already loaded
      // TODO: currently there is a problem if the table is being loaded/created in the same sql-query
      // because the parsing and transformation of subsequent statements will be done before it is actually loaded.
      if (io::StorageManager::getInstance()->exists(table_ref->name)) {
        std::shared_ptr<storage::AbstractTable> table = io::StorageManager::getInstance()->getTable(table_ref->name);
        meta.num_columns = table->columnCount();
        for (field_t i = 0; i != table->columnCount(); ++i) {
          meta.addField(table->metadataAt(i).getName(), table->metadataAt(i).getType());
        }
      }
      break;
    }
    case kTableSelect:
      meta = transformSelectStatement(table_ref->select, task_list);
      break;
    case kTableJoin:
      meta = transformJoinTable(table_ref, task_list);
      break;
    case kTableCrossProduct:
      throwError("Cross table not supported yet");
  }
  
  if (table_ref->getName() != NULL) meta.table_name = table_ref->getName();
  return meta;
}


int SQLStatementTransformer::identifyTableForColumnRef(Expr* col, TransformationResult t1, TransformationResult t2) {
  if (col->hasTable()) {
    // printf("%s %s %s\n", col->table, t1.table_name.c_str(), t2.table_name.c_str());
    if (t1.isTable(col->table)) return 0;
    if (t2.isTable(col->table)) return 1;
    else throwError("Can't find table referenced in column");
  } 

  bool inT1 = t1.containsField(col->name);
  bool inT2 = t2.containsField(col->name);
  if (inT1 && inT2) return -1; // In both tables
  if (inT1) return 0;
  if (inT2) return 1;
  return -2; // In neither table
}


TransformationResult SQLStatementTransformer::transformJoinTable(TableRef* table_ref, task_list_t& task_list) {
  TransformationResult left_res = transformTableRef(table_ref->left, task_list);
  TransformationResult right_res = transformTableRef(table_ref->right, task_list);
  TransformationResult res = ALLOC_TRANSFORMATIONRESULT();
  
  // join_type is not in use atm, only equi join is supported
  auto scan = std::make_shared<JoinScan>(JoinType::EQUI);
  Expr* join_condition = table_ref->join_condition;

  // TODO: allow compound expressions
  if (join_condition->op_type == Expr::SIMPLE_OP && join_condition->op_char == '=') {
    if (join_condition->expr->type != kExprColumnRef || 
        join_condition->expr2->type != kExprColumnRef) {
      throwError("Join condition operator contains unsupported sub-expressions");
    }

    // Detect referenced fields
    int tid1 = identifyTableForColumnRef(join_condition->expr, left_res, right_res);
    int tid2 = identifyTableForColumnRef(join_condition->expr2, left_res, right_res);

    if (tid1 == -1 || tid2 == -1) {
      throwError("Column in join condition found in both tables. Can't be matched securely");
    } else if (tid1 == -2 || tid2 == -2) {
      throwError("Column in join condition can't be found in tables");
    } else if (tid1 == tid2) {
      throwError("Columns in join condition can't be from the same table");
    }

    printf("%d %s:::%d %s\n", tid1, join_condition->expr->name, tid2, join_condition->expr2->name);
    Json::Value pred;
    pred["input_left"] = 0;
    pred["input_right"] = 1;
    if (tid1 == 0) {
      pred["field_left"] = join_condition->expr->name;  
      pred["field_right"] = join_condition->expr2->name;
    } else {
      pred["field_left"] = join_condition->expr2->name;  
      pred["field_right"] = join_condition->expr->name;
    }
    
    scan->addJoinClause<int>(pred);
  } else {
    throwError("Expression in join condition not supported yet");
  }

  scan->addDependency(left_res.last_task);
  scan->addDependency(right_res.last_task);
  appendPlanOp(scan, "ScanJoin", task_list);
  res.first_task = left_res.first_task;
  // TODO: add both column_names vectors here
  res.column_names = left_res.column_names;
  res.last_task = scan;
  return res;
}


SQLStatementTransformer::SQLStatementTransformer(std::string id_prefix) : id_prefix(id_prefix) {
  // Initialize
  last_task_id = 0;
}



} // namespace sql
} // namespace access
} // namespace hyrise