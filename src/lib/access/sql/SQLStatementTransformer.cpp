// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/SQLPredicateTransformer.h"
#include "access/sql/transformation_helper.h"
#include "access/sql/parser/sqlhelper.h"

#include "io/StorageManager.h"

#include "access/expressions/pred_buildExpression.h"

// Operators

#include "access/storage/GetTable.h"

#include "access/MergeTable.h"
#include "access/HashBuild.h"
#include "access/HashJoinProbe.h"
#include "access/SimpleTableScan.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {


/**
 * Constructor of the statement transformer.
 */
SQLStatementTransformer::SQLStatementTransformer(std::string id_prefix) :
  _builder(TaskListBuilder(id_prefix)),
  _select_transformer(nullptr),
  _definition_transformer(nullptr),
  _manipulation_transformer(nullptr) { /* initialize */ }

SQLStatementTransformer::~SQLStatementTransformer() {
  delete _select_transformer;
  delete _definition_transformer;
  delete _manipulation_transformer;
}

SQLSelectTransformer* SQLStatementTransformer::getSelectTransformer() {
  if (_select_transformer == nullptr) _select_transformer = new SQLSelectTransformer(*this);
  return _select_transformer;
}

SQLDataDefinitionTransformer* SQLStatementTransformer::getDataDefinitionTransformer() {
  if (_definition_transformer == nullptr) _definition_transformer = new SQLDataDefinitionTransformer(*this);
  return _definition_transformer;
}
SQLDataManipulationTransformer* SQLStatementTransformer::getDataManipulationTransformer() {
  if (_manipulation_transformer == nullptr) _manipulation_transformer = new SQLDataManipulationTransformer(*this);
  return _manipulation_transformer;
}


/** 
 * Transforms a statement into tasks. 
 * Figures out the type and calls the appropriate transformation method.
 */
TransformationResult SQLStatementTransformer::transformStatement(SQLStatement* stmt) {
  switch (stmt->type()) {
    case kStmtSelect:
      return getSelectTransformer()->transformSelectStatement((SelectStatement*)stmt);
    case kStmtCreate:
      return getDataDefinitionTransformer()->transformCreateStatement((CreateStatement*)stmt);
    case kStmtDrop:
      return getDataDefinitionTransformer()->transformDropStatement((DropStatement*)stmt);
    case kStmtInsert:
      return getDataManipulationTransformer()->transformInsertStatement((InsertStatement*)stmt);
    case kStmtDelete:
      return getDataManipulationTransformer()->transformDeleteStatement((DeleteStatement*)stmt);
    case kStmtUpdate:
      return getDataManipulationTransformer()->transformUpdateStatement((UpdateStatement*)stmt);
    default:
      throwError("Unsupported statement type!\n");
      return {};
  }
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
      meta = getSelectTransformer()->transformSelectStatement(table_ref->select);
      break;
    case kTableJoin:
      meta = transformJoinTable(table_ref);
      break;
    case kTableCrossProduct: {
      // Do a hash join without predicate
      TransformationResult left = transformTableRef(table_ref->list->at(0));
      TransformationResult right = transformTableRef(table_ref->list->at(1));

      auto build = std::make_shared<HashBuild>();
      auto probe = std::make_shared<HashJoinProbe>();
      build->setKey("join");
      build->addDependency(left.last_task);
      probe->addDependency(build);
      probe->addDependency(right.last_task);
      _builder.addPlanOp(build, "HashBuild", meta);
      _builder.addPlanOp(probe, "HashJoinProbe", meta);

      for (uint i = 0; i < right.fields.size(); ++i)
        meta.addField(right.fields[i], right.name, right.data_types[i]);

      for (uint i = 0; i < left.fields.size(); ++i)
        meta.addField(left.fields[i], left.name, left.data_types[i]);
    }
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

    _builder.addPlanOp(build, "HashBuild");
    _builder.addPlanOp(probe, "HashJoinProbe");
    meta.addTask(build);
    meta.addTask(probe);

    // TODO: add data types and table names
      for (uint i = 0; i < right.fields.size(); ++i)
        meta.addField(right.fields[i], right.name, right.data_types[i]);

      for (uint i = 0; i < left.fields.size(); ++i)
        meta.addField(left.fields[i], left.name, left.data_types[i]);
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
  _builder.addPlanOp(get_table, "GetTable");
  meta.addTask(get_table);

  if (validate) _builder.addValidatePositions(meta);

  // Get meta information about the table
  std::shared_ptr<storage::AbstractTable> table = io::StorageManager::getInstance()->getTable(name);
  for (field_t i = 0; i != table->columnCount(); ++i) {
    // Map data-types:
    TableInfo::AbstractDataType type = TableInfo::kUnknown;
    switch (table->metadataAt(i).getType()) {
      case IntegerType:
      case IntegerTypeDelta:
      case IntegerTypeDeltaConcurrent:
      case IntegerNoDictType:
        type = TableInfo::kInteger; break;
      case FloatType:
      case FloatTypeDelta:
      case FloatTypeDeltaConcurrent:
      case FloatNoDictType:
        type = TableInfo::kFloat; break;
      case StringType:
      case StringTypeDelta:
      case StringTypeDeltaConcurrent:
        type = TableInfo::kString; break;
    }

    meta.addField(table->metadataAt(i).getName(), name, type);
  }

  return meta;
}


plan_op_t SQLStatementTransformer::addFilterOpFromExpr(Expr* expr, TransformationResult& meta) {
  // If we have a where clause specified, we need to build a simple table scan over the result
  // Problem: Expression engine only allows comparisons like this: COLUMN = literal
  // we can't have arithmetic sub expressions or expressions consisting of multiple columns
  SQLPredicateTransformer predicate_transformer(meta);
  Json::Value predicates = predicate_transformer.buildPredicatesFromExpr(expr);

  auto scan = std::make_shared<SimpleTableScan>();
  scan->setProducesPositions(true);
  scan->setPredicate(hyrise::access::buildExpression(predicates));
  _builder.addPlanOp(scan, "SimpleTableScan", meta);
  return scan;
}



} // namespace sql
} // namespace access
} // namespace hyrise