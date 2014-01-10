// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MetaData.h"

#include "access/system/QueryParser.h"
#include "access/system/BasicParser.h"

#include "io/StorageManager.h"

#include "storage/storage_types.h"
#include "storage/TableBuilder.h"
#include "storage/ColumnMetadata.h"
#include "storage/Table.h"

#include <iostream>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerTrivialPlanOperation<MetaData>("MetaData");
}

size_t addEntriesForTableToResultTable(std::shared_ptr<const storage::AbstractTable> table, std::string tableName,
                                       std::shared_ptr<storage::AbstractTable> result, size_t row_count) {
  result->resize(result->size() + table->columnCount());
  for (field_t i = 0; i != table->columnCount(); ++i) {
    result->setValue<hyrise_string_t>(result->numberOfColumn("table"), row_count, tableName);
    result->setValue<hyrise_string_t>(result->numberOfColumn("column"), row_count, table->metadataAt(i).getName());
    result->setValue<hyrise_int_t>(result->numberOfColumn("data_type"), row_count, table->metadataAt(i).getType());

    ++row_count;
  }
  return row_count;
}

void MetaData::executePlanOperation() {
 
  storage::TableBuilder::param_list list;
  list.append().set_type("STRING").set_name("table");
  list.append().set_type("STRING").set_name("column");
  list.append().set_type("INTEGER").set_name("data_type"); //output as string?
  auto meta_data = storage::TableBuilder::build(list);

  size_t row_count = 0;
  const auto &storageManager = io::StorageManager::getInstance();
  const auto& loaded_tables = storageManager->all();

  if (input.numberOfTables() == 0) {

    for (const auto & tableName: storageManager->getTableNames()) {
      auto table = storageManager->getTable(tableName);
      row_count = addEntriesForTableToResultTable(table, tableName, meta_data, row_count);
    }

  } else {

    for (size_t i = 0; i < input.numberOfTables(); ++i) {
      
      auto inputTable = input.getTable(i);
      std::string tableName = "unknown/temporary";

      for (const auto& table: loaded_tables) {
        if (table.second == inputTable) {
          tableName = table.first;
          break;
        }
      }

      row_count = addEntriesForTableToResultTable(inputTable, tableName, meta_data, row_count);
    }
  }

  addResult(meta_data);
}

}
}
