// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/predicate_helper.h"
#include "access/sql/transformation_helper.h"
#include "access/sql/parser/sqlhelper.h"

#include "io/StorageManager.h"

#include "access/expressions/pred_buildExpression.h"

// Operators
#include "access/storage/TableLoad.h"
#include "access/storage/GetTable.h"
#include "access/HashBuild.h"
#include "access/HashJoinProbe.h"
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



/**
 * Constructor of the statement transformer.
 */
SQLStatementTransformer::SQLStatementTransformer(std::string id_prefix) : _id_prefix(id_prefix) {
  // Initialize
  _last_task_id = 0;
}

/** 
 * Transforms a statement into tasks. 
 * Figures out the type and calls the appropriate transformation method.
 */
TransformationResult SQLStatementTransformer::transformStatement(Statement* stmt) {
  switch (stmt->type) {
    case kStmtSelect: printSelectStatementInfo((SelectStatement*)stmt, 0); return transformSelectStatement((SelectStatement*)stmt);
    case kStmtCreate: return transformCreateStatement((CreateStatement*)stmt);
    default: throwError("Unsupported statement type!\n");
  }
  return ALLOC_TRANSFORMATIONRESULT();
}

/** 
 * Transforms a create statement into tasks
 */
TransformationResult SQLStatementTransformer::transformCreateStatement(CreateStatement* stmt) {
  if (io::StorageManager::getInstance()->exists(stmt->table_name)) {
    // TODO: This should be handled by the plan operator that is used
    throwError("Table already exists");
  }

  std::shared_ptr<TableLoad> table_load = std::make_shared<TableLoad>();
  table_load->setTableName(std::string(stmt->table_name));
  table_load->setFileName(std::string(stmt->file_path));
  appendPlanOp(table_load, "TableLoad");

  std::shared_ptr<NoOp> no_op = std::make_shared<NoOp>();
  no_op->addDependency(table_load);
  appendPlanOp(no_op, "NoOp");
  
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
  meta.first_task = table_load;
  meta.last_task = no_op;
  return meta;
}

/** 
 * Transforms a select statement into tasks
 */
TransformationResult SQLStatementTransformer::transformSelectStatement(SelectStatement* stmt) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();

  // SQL Order of Operations: http://www.bennadel.com/blog/70-sql-query-order-of-operations.htm
  // 1. FROM clause
  // 2. WHERE clause
  // 3. GROUP BY clause
  // 4. HAVING clause (TODO)
  // 5. SELECT clause
  // 6. ORDER BY clause (TODO)


  // FROM clause
  TransformationResult table_res = transformTableRef(stmt->from_table);
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
    appendPlanOp(scan, "SimpleTableScan");
    meta.last_task = scan;
  } 

  // GROUP BY clause
  if (stmt->group_by != nullptr) {
    TransformationResult group_res = transformGroupByClause(stmt);
    meta.fields = group_res.fields;
    meta.data_types = group_res.data_types;
    meta.last_task = group_res.last_task;
  }

  // SELECT clause
  // If there is only SELECT * specified we don't have to do a projection
  if (stmt->select_list->at(0)->type != kExprStar || stmt->select_list->size() > 1) {
    TransformationResult res = transformSelectionList(stmt, meta);
    meta.fields = res.fields;
    meta.data_types = res.data_types;
    meta.last_task = res.last_task;
  }

  // ORDER BY clause
  if (stmt->order != nullptr) {
    throwError("Order By not supported yet");
  }

  // LIMIT clause
  if (stmt->limit != nullptr) {
    // Projection Scan is the only op I could find that supports limit
    // Problem: Offset not supported by operator
    if (stmt->limit->offset != kNoOffset) throwError("Offset not supported yet");
    
    std::shared_ptr<ProjectionScan> scan = std::make_shared<ProjectionScan>();
    for (std::string column : meta.fields) scan->addField(column);
    scan->setLimit(stmt->limit->limit);
    scan->addDependency(meta.last_task);
    appendPlanOp(scan, "LimitScan");
    meta.last_task = scan;
  }

  // UNION
  // Problem: UnionScan can only work on the same table and only if positions have been produced
  if (stmt->union_select != nullptr) {
    TransformationResult t_res = transformSelectStatement(stmt->union_select);
    std::shared_ptr<UnionScan> scan = std::make_shared<UnionScan>();
    scan->addDependency(meta.last_task);
    scan->addDependency(t_res.last_task);
    appendPlanOp(scan, "UnionScan");
    meta.last_task = scan;
  }

  return meta;
}

/**
 * Transform the group by clause of a SelectStatement.
 *
 * @param[in]  stmt       SelectStatement thats selections list will be transformed
 * @return  Information about the result of the transformation
 */
TransformationResult SQLStatementTransformer::transformGroupByClause(SelectStatement* stmt) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
  task_t input_task = _task_list.back();
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
  appendPlanOp(hash, "HashBuild");

  scan->addDependency(input_task);
  scan->addDependency(hash);
  appendPlanOp(scan, "GroupByScan");

  meta.first_task = hash;
  meta.last_task = scan;

  return meta;
}

/**
 * Transform the selection list of a SelectStatement into a ProjectionScan task.
 *
 * @param[in]  stmt       SelectStatement thats selections list will be transformed
 * @param[in]  info       Information about the source for the projection
 * @return  Information about the result of the transformation
 */
TransformationResult SQLStatementTransformer::transformSelectionList(hsql::SelectStatement* stmt, TransformationResult info) {
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
        for (size_t i = 0; i < info.numColumns(); ++i) {
          scan->addField(info.fields[i]);
          res.addField(info.fields[i]);
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
  appendPlanOp(scan, "ProjectionScan");
  res.last_task = scan;
  return res;
}

/**
 * Transform the table ref into the equivalent hyrise taskts
 *
 * @param[in]  table_ref  TableRef that will be transformed
 * @return  Information about the result of the transformation
 */
TransformationResult SQLStatementTransformer::transformTableRef(TableRef* table_ref) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();

  switch (table_ref->type) {
    case kTableName: {
      if (!io::StorageManager::getInstance()->exists(table_ref->name)) throwError("Table doesn't exist", table_ref->name);

      std::shared_ptr<GetTable> get_table = std::make_shared<GetTable>(table_ref->name);
      appendPlanOp(get_table, "GetTable");
      meta.first_task = get_table;
      meta.last_task = get_table;

      // Get table definition, if it is already loaded
      // TODO: currently there is a problem if the table is being loaded/created in the same sql-query
      // because the parsing and transformation of subsequent statements will be done before it is actually loaded.
      if (io::StorageManager::getInstance()->exists(table_ref->name)) {
        std::shared_ptr<storage::AbstractTable> table = io::StorageManager::getInstance()->getTable(table_ref->name);
        for (field_t i = 0; i != table->columnCount(); ++i) {
          meta.addField(table->metadataAt(i).getName(), table->metadataAt(i).getType());
        }
      }
      break;
    }
    case kTableSelect:
      meta = transformSelectStatement(table_ref->select);
      break;
    case kTableJoin:
      meta = transformJoinTable(table_ref);
      break;
    case kTableCrossProduct:
      throwError("Cross table not supported yet");
  }
  
  if (table_ref->getName() != NULL) meta.table_name = table_ref->getName();
  // LOG_META(meta);
  return meta;
}

/**
 * Transforms a join table
 * In this method we try to find out which join algorithm is appropriate for the query.
 * At the moment we simply use HashJoin.
 */
TransformationResult SQLStatementTransformer::transformJoinTable(TableRef* table) {
  TransformationResult res;
  res = transformHashJoin(table);
  return res;
}

/**
 * Transforms a join table into a JoinScan operator
 */
TransformationResult SQLStatementTransformer::transformScanJoin(TableRef* table) {
  TransformationResult res = ALLOC_TRANSFORMATIONRESULT();
  JoinDefinition* join = table->join;
  Expr* condition = join->condition;
  if (join->type != kJoinInner) throwError("JoinScan only supports inner join");

  // Only Equi Join is supported by JoinScan
  // TODO: allow compound expressions
  if (condition->isSimpleOp('=')) {
    if (!condition->expr->isType(kExprColumnRef) || !condition->expr2->isType(kExprColumnRef)) {
      throwError("Join condition operator contains unsupported sub-expressions");
    }
    auto scan = std::make_shared<JoinScan>(JoinType::EQUI);

    TransformationResult left = transformTableRef(join->left);
    TransformationResult right = transformTableRef(join->right);

    // Detect referenced fields
    int tid1 = identifyTableForColumnRef(condition->expr, left, right);
    int tid2 = identifyTableForColumnRef(condition->expr2, left, right);

    if (tid1 == -1 || tid2 == -1) {
      throwError("Column in join condition found in both tables. Can't be matched securely");
    } else if (tid1 == -2 || tid2 == -2) {
      throwError("Column in join condition can't be found in tables");
    } else if (tid1 == tid2) {
      throwError("Columns in join condition can't be from the same table");
    }

    std::string left_column = (tid1 == 0) ? condition->expr->name : condition->expr2->name;
    std::string right_column = (tid1 == 1) ? condition->expr->name : condition->expr2->name;

    Json::Value pred;
    pred["input_left"] = 0;
    pred["input_right"] = 1;
    pred["field_left"] = left_column;
    pred["field_right"] = right_column;
    scan->addJoinClause<int>(pred);

    scan->addDependency(left.last_task);
    scan->addDependency(right.last_task);
    appendPlanOp(scan, "ScanJoin");
    res.first_task = left.first_task;
    res.last_task = scan;
    res.fields = combineVectors<std::string>(left.fields, right.fields);
  } else {
    throwError("Expression in join condition not supported yet");
  }
  return res;
}

/**
 * Transforms a join table into a HashJoin operator
 */
TransformationResult SQLStatementTransformer::transformHashJoin(TableRef* table) {
  TransformationResult res = ALLOC_TRANSFORMATIONRESULT();
  JoinDefinition* join = table->join;
  Expr* condition = join->condition;

  if (condition->isType(kExprOperator) && condition->isSimpleOp('=')) {
    if (!condition->expr->isType(kExprColumnRef) || !condition->expr2->isType(kExprColumnRef)) {
      throwError("Join condition can only contain column references");
    }

    TransformationResult left = transformTableRef(join->left);
    TransformationResult right = transformTableRef(join->right);

    // Detect referenced fields
    int tid1 = identifyTableForColumnRef(condition->expr, left, right);
    int tid2 = identifyTableForColumnRef(condition->expr2, left, right);

    if (tid1 == -1 || tid2 == -1) {
      std::string err_detail = (tid1 == -1) ? condition->expr->name : condition->expr2->name;
      throwError("Column in join condition found in both tables", err_detail);
    } else if (tid1 == -2 || tid2 == -2) {
      std::string err_detail = (tid1 == -2) ? condition->expr->name : condition->expr2->name;
      throwError("Column in join condition can't be found in tables", err_detail);
    } else if (tid1 == tid2) {
      throwError("Columns in join condition can't be from the same table");
    }

    std::string left_column = (tid1 == 0) ? condition->expr->name : condition->expr2->name;
    std::string right_column = (tid1 == 1) ? condition->expr->name : condition->expr2->name;


    auto build = std::make_shared<HashBuild>();
    build->setKey("join");
    build->addField(left_column);

    auto probe = std::make_shared<HashJoinProbe>();
    probe->addField(right_column);

    build->addDependency(left.last_task);
    probe->addDependency(build);
    probe->addDependency(right.last_task);

    appendPlanOp(build, "HashBuild");
    appendPlanOp(probe, "HashJoinProbe");
    res.first_task = build;
    res.last_task = probe;
    for (std::string col : right.fields) res.addField(col);
    for (std::string col : left.fields) res.addField(col);
  } else {
    // TODO: support multi equi join, HashJoin seems to allow that
    throwError("Only support single equi join");
  }
  return res;
}




} // namespace sql
} // namespace access
} // namespace hyrise