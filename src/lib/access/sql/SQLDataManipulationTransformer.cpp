// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/sql/SQLDataManipulationTransformer.h"
#include "access/sql/SQLStatementTransformer.h"

#include "access/Delete.h"
#include "access/PosUpdateScan.h"
#include "access/InsertScan.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

SQLDataManipulationTransformer::SQLDataManipulationTransformer(SQLStatementTransformer& server) :
  _server(server),
  _builder(server.getTaskListBuilder()) {}

SQLDataManipulationTransformer::~SQLDataManipulationTransformer() {}



TransformationResult SQLDataManipulationTransformer::transformInsertStatement(InsertStatement* insert) {
  // First get the table into which we want to insert (Important: no validation of positions)
  TransformationResult meta = _server.addGetTable(insert->table_name, false);
  
  if (insert->type == InsertStatement::kInsertValues) {
    if (insert->columns != nullptr)
      _server.throwError("Specifying columns explicitly is not supported");

    // Check if number of values and number of columns in table match
    if (meta.fields.size() != insert->values->size()) 
      _server.throwError("Numbers of values don't match number of columns in table");

    // Check if types of values and columns match
    for (size_t i = 0; i < meta.fields.size(); ++i) {
      Expr* expr = insert->values->at(i);
      switch (meta.data_types[i]) {
        case TableInfo::kInteger: // Expect integer, allow integer
          if (!(expr->isType(kExprLiteralInt)))
            _server.throwError("Type of value doesn't match type of column", "value #" + std::to_string(i));
          break;
        case TableInfo::kFloat: // Expect Float, allow float or integer
          if (!(expr->isType(kExprLiteralInt) || expr->isType(kExprLiteralFloat)))
            _server.throwError("Type of value doesn't match type of column", "value #" + std::to_string(i));
          break;
        case TableInfo::kString: // Expect String, allow all
          break;
        default:
          // printf("ColumnType: %u\n", meta.data_types[i]);
          _server.throwError("Unexpected type of column");
      }
    }

   
    auto insert_scan = std::make_shared<InsertScan>();
    std::vector<Json::Value> data;
    for (Expr* expr : insert->values->vector()) { 
      switch (expr->type) {
        case kExprLiteralFloat: data.push_back(expr->fval); break;
        case kExprLiteralInt: data.push_back(expr->ival); break;
        case kExprLiteralString: data.push_back(expr->name); break;
        default: _server.throwError("Unsupported value type in insert!");
      }
    }
    insert_scan->addDataRow(data);
    _builder.addPlanOp(insert_scan, "InsertScan", meta);

  } else {
    _server.throwError("Unsupported insert method");
  }

  _builder.addCommit(meta);
  _builder.addNoOp(meta);
  return meta;
}



TransformationResult SQLDataManipulationTransformer::transformDeleteStatement(DeleteStatement* del) {
  TransformationResult meta = _server.addGetTable(del->table_name, true);

  if (del->expr != nullptr) {
    // Delete whatever matches the expression
    auto filter = _server.addFilterOpFromExpr(del->expr, meta);
  }

  auto delete_op = std::make_shared<DeleteOp>();

  _builder.addPlanOp(delete_op, "DeleteOp", meta);
  _builder.addCommit(meta);
  _builder.addNoOp(meta);
  return meta;
}



TransformationResult SQLDataManipulationTransformer::transformUpdateStatement(UpdateStatement* update) {
  TransformationResult meta = _server.addGetTable(update->table->name, true);

  if (update->where == nullptr) {
    _server.throwError("Update without WHERE clause not supported yet");
  } else {
    auto filter = _server.addFilterOpFromExpr(update->where, meta);
  }

  auto update_op = std::make_shared<PosUpdateScan>();

  Json::Value update_data;
  for (UpdateClause* clause : update->updates->vector()) {
    switch (clause->value->type) {
      case kExprLiteralInt: update_data[clause->column] = clause->value->ival; break;
      case kExprLiteralFloat: update_data[clause->column] = clause->value->fval; break;
      case kExprLiteralString: update_data[clause->column] = clause->value->name; break;
      default:
        _server.throwError("Unsupported Expr type in update clause");    
    }  
  }
  update_op->setRawData(update_data);

  _builder.addPlanOp(update_op, "PosUpdateScan", meta);
  _builder.addCommit(meta);
  _builder.addNoOp(meta);
  return meta;
}




} // namespace sql
} // namespace access
} // namespace hyrise