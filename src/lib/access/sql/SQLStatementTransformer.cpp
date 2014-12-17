// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/predicate_helper.h"
#include "access/sql/transformation_helper.h"
#include "access/sql/parser/sqlhelper.h"

#include "io/StorageManager.h"

#include "access/expressions/pred_buildExpression.h"

// Operators
#include "access/tx/ValidatePositions.h"

#include "access/storage/TableLoad.h"
#include "access/storage/TableUnload.h"
#include "access/storage/JsonTable.h"
#include "access/storage/GetTable.h"
#include "access/storage/SetTable.h"

#include "access/MergeTable.h"
#include "access/HashBuild.h"
#include "access/HashJoinProbe.h"
#include "access/SimpleTableScan.h"
#include "access/UnionScan.h"
#include "access/Delete.h"
#include "access/PosUpdateScan.h"
#include "access/JoinScan.h"
#include "access/ProjectionScan.h"
#include "access/GroupByScan.h"
#include "access/NoOp.h"
#include "access/InsertScan.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

/**
 * Constructor of the statement transformer.
 */
SQLStatementTransformer::SQLStatementTransformer(std::string id_prefix) :
  _id_prefix(id_prefix),
  _last_task_id(0) {
  // Initialize
}

/** 
 * Transforms a statement into tasks. 
 * Figures out the type and calls the appropriate transformation method.
 */
TransformationResult SQLStatementTransformer::transformStatement(SQLStatement* stmt) {
  switch (stmt->type()) {
    case kStmtSelect:
      // printSelectStatementInfo((SelectStatement*)stmt, 0);
      return transformSelectStatement((SelectStatement*)stmt); break;
    case kStmtCreate:
      return transformCreateStatement((CreateStatement*)stmt); break;
    case kStmtInsert:
      return transformInsertStatement((InsertStatement*)stmt); break;
    case kStmtDelete:
      return transformDeleteStatement((DeleteStatement*)stmt); break;
    case kStmtUpdate:
      return transformUpdateStatement((UpdateStatement*)stmt); break;
    case kStmtDrop:
      return transformDropStatement((DropStatement*)stmt); break;
    default:
      throwError("Unsupported statement type!\n");
      return {};
  }
}

TransformationResult SQLStatementTransformer::transformDropStatement(DropStatement* drop) {
  TransformationResult meta = {};

  if (drop->type == DropStatement::kTable) {
    if (!io::StorageManager::getInstance()->exists(drop->name)) {
      throwError("Can't drop table. It doesn't exist.", drop->name);
    }

    auto drop_op = addNewPlanOp<TableUnload>("TableUnload");
    drop_op->setTableName(drop->name);
    meta.addTask(drop_op);

  } else {
    throwError("Drop type not supported");
  }

  addCommit(meta);
  return meta;
}


TransformationResult SQLStatementTransformer::transformUpdateStatement(UpdateStatement* update) {
  TransformationResult meta = addGetTable(update->table->name, true);

  if (update->where == nullptr) {
    throwError("Update without WHERE clause not supported yet");
  } else {
    auto filter = addFilterOpFromExpr(update->where, meta);
  }

  auto update_op = addNewPlanOp<PosUpdateScan>("PosUpdateScan", meta);

  Json::Value update_data;
  for (UpdateClause* clause : update->updates->vector()) {
    switch (clause->value->type) {
      case kExprLiteralInt: update_data[clause->column] = clause->value->ival; break;
      case kExprLiteralFloat: update_data[clause->column] = clause->value->fval; break;
      case kExprLiteralString: update_data[clause->column] = clause->value->name; break;
      default:
        throwError("Unsupported Expr type in update clause");    
    }  
  }
  update_op->setRawData(update_data);

  addCommit(meta);
  addNewPlanOp<NoOp>("NoOp", meta);
  return meta;
}

/** 
 * Transforms a create statement into tasks
 */
TransformationResult SQLStatementTransformer::transformCreateStatement(CreateStatement* create) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();

  if (io::StorageManager::getInstance()->exists(create->table_name)) {
    if (create->if_not_exists) {
      // Table already exists so skip this statement
      addNewPlanOp<NoOp>("NoOp", meta);
      return meta;
    } else {
      // Table already exists -> throw error
      throwError("Table already exists", create->table_name);
    }
  }

  if (create->type == CreateStatement::kTableFromTbl) {
    // Create TableLoad
    auto table_load = addNewPlanOp<TableLoad>("TableLoad", meta);
    table_load->setTableName(std::string(create->table_name));
    table_load->setFileName(std::string(create->file_path));

  } else if (create->type == CreateStatement::kTable) {
    // Create and SetTable
    std::vector<std::string> names;
    std::vector<std::string> types;
    std::vector<unsigned> groups;

    for (ColumnDefinition* def : create->columns->vector()) {
      names.push_back(def->name);
      groups.push_back(1);
      switch (def->type) {
        case ColumnDefinition::INT: types.push_back("INTEGER"); break;
        case ColumnDefinition::TEXT: types.push_back("STRING"); break;
        case ColumnDefinition::DOUBLE: types.push_back("FLOAT"); break;
      }
    }

    // auto json_table = std::make_shared<JsonTable>();
    auto json_table = addNewPlanOp<JsonTable>("JsonTable");
    json_table->setNames(names);
    json_table->setTypes(types);
    json_table->setGroups(groups);
    json_table->setUseStore(true);
    
    // Give it a name and persist it within Hyrise storage manager
    auto set_table = std::make_shared<SetTable>(create->table_name);
    set_table->addDependency(json_table);
    addPlanOp(set_table, "SetTable");

    meta.addTask(json_table);
    meta.addTask(set_table);

  } else {
    throwError("Unsupported create type!");
  }

  // Add No op, so we don't send data back
  addCommit(meta);
  addNewPlanOp<NoOp>("NoOp", meta);
  return meta;
}



TransformationResult SQLStatementTransformer::transformDeleteStatement(DeleteStatement* del) {
  TransformationResult meta = addGetTable(del->table_name, true);

  if (del->expr != nullptr) {
    // Delete whatever matches the expression
    auto filter = addFilterOpFromExpr(del->expr, meta);
  }

  addNewPlanOp<DeleteOp>("DeleteOp", meta);
  addCommit(meta);
  addNewPlanOp<NoOp>("NoOp", meta);
  return meta;
}



TransformationResult SQLStatementTransformer::transformInsertStatement(InsertStatement* insert) {
  // First get the table into which we want to insert (Important: no validation of positions)
  TransformationResult meta = addGetTable(insert->table_name, false);
  
  if (insert->type == InsertStatement::kInsertValues) {
    if (insert->columns != nullptr)
      throwError("Specifying columns explicitely is not supported");

    // Check if number of values and number of columns in table match
    if (meta.fields.size() != insert->values->size()) 
      throwError("Numbers of values don't match number of columns in table");

    // Check if types of values and columns match
    for (size_t i = 0; i < meta.fields.size(); ++i) {
      Expr* expr = insert->values->at(i);
      switch (meta.data_types[i]) {
        case IntegerType: // Expect integer, allow integer
        case IntegerTypeDelta:
          if (!(expr->isType(kExprLiteralInt)))
            throwError("Type of value doesn't match type of column", "value #" + std::to_string(i));
          break;
        case FloatType: // Expect Float, allow float or integer
        case FloatTypeDelta:
          if (!(expr->isType(kExprLiteralInt) || expr->isType(kExprLiteralFloat)))
            throwError("Type of value doesn't match type of column", "value #" + std::to_string(i));
          break;
        case StringType: // Expect String, allow all
        case StringTypeDelta:
          break;
        default:
          // printf("ColumnType: %u\n", meta.data_types[i]);
          throwError("Unexpected type of column");
      }
    }

   
    auto insert_scan = addNewPlanOp<InsertScan>("InsertScan", meta);
    std::vector<Json::Value> data;
    for (Expr* expr : insert->values->vector()) { 
      switch (expr->type) {
        case kExprLiteralFloat: data.push_back(expr->fval); break;
        case kExprLiteralInt: data.push_back(expr->ival); break;
        case kExprLiteralString: data.push_back(expr->name); break;
        default: throwError("Unsupported value type in insert!");
      }
    }
    insert_scan->addDataRow(data);
  } else {
    throwError("Unsupported insert method");
  }

  addCommit(meta);
  addNewPlanOp<NoOp>("NoOp", meta);
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
    auto scan = addFilterOpFromExpr(stmt->where_clause, meta);
  } 

  // GROUP BY clause
  if (stmt->group_by != nullptr) {
    TransformationResult group_res = transformGroupByClause(stmt);
    meta.append(group_res);
  }

  // SELECT clause
  // If there is only SELECT * specified we don't have to do a projection
  if (stmt->select_list->at(0)->type != kExprStar || stmt->select_list->size() > 1) {
    TransformationResult res = transformSelectionList(stmt, meta);
    meta.append(res);
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
    
    auto scan = addNewPlanOp<ProjectionScan>("LimitScan", meta);
    for (std::string column : meta.fields) scan->addField(column);
    scan->setLimit(stmt->limit->limit);
  }

  // UNION
  // Problem: UnionScan can only work on the same table and only if positions have been produced
  if (stmt->union_select != nullptr) {
    TransformationResult sub_select = transformSelectStatement(stmt->union_select);
    auto scan = addNewPlanOp<UnionScan>("UnionScan", meta);
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
      std::string column_name = (expr->alias == nullptr) ? buildFunctionRefColumnName(expr) : expr->alias;
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
  addPlanOp(hash, "HashBuild");

  scan->addDependency(input_task);
  scan->addDependency(hash);
  addPlanOp(scan, "GroupByScan");

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
TransformationResult SQLStatementTransformer::transformSelectionList(SelectStatement* stmt, const TransformationResult& input) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
  // If we are not selecting everything, we have to perform a projection scan
  // Problem: can't select literals with this operator
  auto scan = std::make_shared<ProjectionScan>();

  for (Expr* expr : stmt->select_list->vector()) {
    switch (expr->type) {
      case kExprColumnRef:
        scan->addField(expr->name);
        meta.addField(expr->name);
        break;
      case kExprStar:
        // This can only work if there is meta information available about the table
        for (size_t i = 0; i < input.numColumns(); ++i) {
          scan->addField(input.fields[i]);
          meta.addField(input.fields[i]);
        }
        break;
      case kExprFunctionRef: {
        // Assumption: We had a group by previously
        std::string name = (expr->alias == nullptr) ? buildFunctionRefColumnName(expr) : expr->alias;
        scan->addField(name);
        meta.addField(name);
        break;
      }
      default: 
        throwError("Expression type not supported in SELECT clause");
        break;
    }
  }

  scan->addDependency(input.last_task);
  meta.addTask(addPlanOp(scan, "ProjectionScan"));
  return meta;
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
    case kTableName:
      meta = addGetTable(table_ref->name, true);
      break;
    case kTableSelect:
      meta = transformSelectStatement(table_ref->select);
      break;
    case kTableJoin:
      meta = transformJoinTable(table_ref);
      break;
    case kTableCrossProduct:
      throwError("Cross table not supported yet");
  }
  
  if (table_ref->getName() != nullptr) meta.name = table_ref->getName();
  // LOG_META(meta);
  return meta;
}

/**
 * Transforms a join table
 * In this method we try to find out which join algorithm is appropriate for the query.
 * At the moment we simply use HashJoin.
 */
TransformationResult SQLStatementTransformer::transformJoinTable(TableRef* table) {
  TransformationResult meta;
  meta = transformHashJoin(table);
  return meta;
}

/**
 * Transforms a join table into a JoinScan operator
 */
TransformationResult SQLStatementTransformer::transformScanJoin(TableRef* table) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
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
    addPlanOp(scan, "ScanJoin");
    meta.addTask(left.first_task);
    meta.addTask(scan);
    meta.fields = combineVectors<std::string>(left.fields, right.fields);
  } else {
    throwError("Expression in join condition not supported yet");
  }
  return meta;
}

/**
 * Transforms a join table into a HashJoin operator
 */
TransformationResult SQLStatementTransformer::transformHashJoin(TableRef* table) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();
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

    addPlanOp(build, "HashBuild");
    addPlanOp(probe, "HashJoinProbe");
    meta.addTask(build);
    meta.addTask(probe);
    for (std::string col : right.fields) meta.addField(col);
    for (std::string col : left.fields) meta.addField(col);
  } else {
    // TODO: support multi equi join, HashJoin seems to allow that
    throwError("Only support single equi join");
  }
  return meta;
}


/**
 * Create a task to get the table specified by the name
 */
TransformationResult SQLStatementTransformer::addGetTable(std::string name, bool validate) {
  if (!io::StorageManager::getInstance()->exists(name)) throwError("Table doesn't exist", name);
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();

  auto get_table = std::make_shared<GetTable>(name);
  addPlanOp(get_table, "GetTable");
  meta.addTask(get_table);

  if (validate) addNewPlanOp<ValidatePositions>("ValidatePositions", meta);

  // Get meta information about the table
  std::shared_ptr<storage::AbstractTable> table = io::StorageManager::getInstance()->getTable(name);
  for (field_t i = 0; i != table->columnCount(); ++i) {
    meta.addField(table->metadataAt(i).getName(), table->metadataAt(i).getType());
  }

  return meta;
}


plan_op_t SQLStatementTransformer::addFilterOpFromExpr(Expr* expr, TransformationResult& meta) {
  // If we have a where clause specified, we need to build a simple table scan over the result
  // Problem: Expression engine only allows comparisons like this: COLUMN = literal
  // we can't have arithmetic sub expressions or expressions consisting of multiple columns
  // TODO: Supply the data types to the expression builder from meta information
  Json::Value predicates;
  buildPredicatesFromSQLExpr(expr, predicates);

  auto scan = addNewPlanOp<SimpleTableScan>("SimpleTableScan", meta);
  scan->setProducesPositions(true);
  scan->setPredicate(hyrise::access::buildExpression(predicates));
  return scan;
}


template<typename _T>
std::shared_ptr<_T> SQLStatementTransformer::addNewPlanOp(std::string name, task_t dependency) {
  auto op = std::make_shared<_T>();
  if (dependency != nullptr) op->addDependency(dependency);
  addPlanOp(op, name);
  return op;
}


template<typename _T>
std::shared_ptr<_T> SQLStatementTransformer::addNewPlanOp(std::string name, TransformationResult& meta) {
  auto op = addNewPlanOp<_T>(name, meta.last_task);
  meta.addTask(op);
  return op;
}


plan_op_t SQLStatementTransformer::addPlanOp(plan_op_t task, std::string name) {
  task->setPlanOperationName(name);
  task->setOperatorId(nextTaskId() + " " + name);
  _task_list.push_back(task);
  return task;
}

inline std::shared_ptr<Commit> SQLStatementTransformer::addCommit(TransformationResult& meta) { 
  _has_commit = true;
  return addNewPlanOp<Commit>("Commit", meta);
}

std::string SQLStatementTransformer::nextTaskId() {
  return _id_prefix + std::to_string(++_last_task_id);
}


} // namespace sql
} // namespace access
} // namespace hyrise