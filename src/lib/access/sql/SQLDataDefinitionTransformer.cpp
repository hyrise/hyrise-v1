// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/sql/SQLDataDefinitionTransformer.h"
#include "access/sql/SQLStatementTransformer.h"
#include "access/sql/transformation_helper.h"

#include "io/StorageManager.h"

#include "access/storage/TableLoad.h"
#include "access/storage/TableUnload.h"
#include "access/storage/JsonTable.h"
#include "access/storage/SetTable.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {

SQLDataDefinitionTransformer::SQLDataDefinitionTransformer(SQLStatementTransformer& server) :
  _server(server),
  _builder(server.getTaskListBuilder()) {}

SQLDataDefinitionTransformer::~SQLDataDefinitionTransformer() {}

/** 
 * Transforms a create statement into tasks
 */
TransformationResult SQLDataDefinitionTransformer::transformCreateStatement(CreateStatement* create) {
  TransformationResult meta = ALLOC_TRANSFORMATIONRESULT();

  if (io::StorageManager::getInstance()->exists(create->table_name)) {
    if (create->if_not_exists) {
      // Table already exists so skip this statement
      _builder.addNoOp(meta);
      return meta;
    } else {
      // Table already exists -> throw error
      _server.throwError("Table already exists", create->table_name);
    }
  }

  if (create->type == CreateStatement::kTableFromTbl) {
    // Create TableLoad
    auto table_load = std::make_shared<TableLoad>();//addNewPlanOp<TableLoad>("TableLoad", meta);
    table_load->setTableName(std::string(create->table_name));
    table_load->setFileName(std::string(create->file_path));
    _builder.addPlanOp(table_load, "TableLoad", meta);

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

    auto json_table = std::make_shared<JsonTable>();
    json_table->setNames(names);
    json_table->setTypes(types);
    json_table->setGroups(groups);
    json_table->setUseStore(true);
    _builder.addPlanOp(json_table, "JsonTable", meta);
    
    // Give it a name and persist it within Hyrise storage manager
    auto set_table = std::make_shared<SetTable>(create->table_name);
    set_table->addDependency(json_table);
    _builder.addPlanOp(set_table, "SetTable");

    meta.addTask(json_table);
    meta.addTask(set_table);

  } else {
    _server.throwError("Unsupported create type!");
  }

  // Add No op, so we don't send data back
  _builder.addCommit(meta);
  _builder.addNoOp(meta);
  return meta;
}



TransformationResult SQLDataDefinitionTransformer::transformDropStatement(DropStatement* drop) {
  TransformationResult meta = {};

  if (drop->type == DropStatement::kTable) {
    if (!io::StorageManager::getInstance()->exists(drop->name)) {
      _server.throwError("Can't drop table. It doesn't exist.", drop->name);
    }

    auto drop_op = std::make_shared<TableUnload>();
    drop_op->setTableName(drop->name);
    _builder.addPlanOp(drop_op, "TableUnload", meta);

  } else {
    _server.throwError("Drop type not supported");
  }

  _builder.addCommit(meta);
  return meta;
}



} // namespace sql
} // namespace access
} // namespace hyrise