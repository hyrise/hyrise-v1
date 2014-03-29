// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <set>

#include "access/InsertScan.h"
#include "access/system/ResponseTask.h"

#include "json_converters.h"

#include "helper/vector_helpers.h"
#include "helper/checked_cast.h"
#include "helper/stringhelpers.h"

#include "io/TransactionManager.h"
#include "io/logging.h"
#include "io/ResourceManager.h"
#include "io/StorageManager.h"

#include "storage/Store.h"
#include "storage/Serial.h"
#include "storage/meta_storage.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<InsertScan>("InsertScan");
}

InsertScan::~InsertScan() {}

void InsertScan::executePlanOperation() {
  const auto& c_store = checked_pointer_cast<const storage::Store>(input.getTable(0));
  // Cast the constness away
  auto store = std::const_pointer_cast<storage::Store>(c_store);

  const size_t columnCount = store->columnCount();
  const size_t rowCount = _data ? _data->size() : _raw_data.size();
  const auto& writeArea = store->appendToDelta(rowCount);
  const size_t firstPosition = store->getMainTable()->size() + writeArea.first;
  auto& mods = tx::TransactionManager::getInstance()[_txContext.tid];

  if (!_data) {
    // determine serial [=autoincrement] fields in store
    auto& resMgr = io::ResourceManager::getInstance();
    std::set<field_t> serialFields;
    for (size_t c = 0; c < columnCount; ++c) {
      auto serial_name = std::to_string(store->getUuid()) + "_" + store->nameOfColumn(c);
      if (resMgr.exists(serial_name)) {
        serialFields.insert(c);
      }
    }

    // extend _raw_data with serial fields (if any)
    if (!serialFields.empty()) {
      std::vector<std::vector<Json::Value>> extended_raw_data(rowCount, std::vector<Json::Value>(columnCount));
      for (size_t r = 0; r < rowCount; ++r) {
        size_t columnOffset = 0;
        for (size_t c = 0; c < columnCount; ++c) {
          if (serialFields.count(c) != 0) {
            std::string serialName = std::to_string(store->getUuid()) + "_" + store->nameOfColumn(c);
            Json::Value v(resMgr.get<storage::Serial>(serialName)->next());
            extended_raw_data[r][c] = v;
            ++columnOffset;
          } else {
            extended_raw_data[r][c] = _raw_data[r][c - columnOffset];
          }
        }
      }
      _raw_data = extended_raw_data;
    }

    for (size_t i = 0; i < rowCount; ++i) {
      store->copyRowToDeltaFromJSONVector(_raw_data[i], writeArea.first + i, _txContext.tid);

      // Update delta indices
      store->addRowToDeltaIndices(firstPosition + i);

      mods.insertPos(store, firstPosition + i);

#ifdef PERSISTENCY_BUFFEREDLOGGER
      std::vector<ValueId> vids = store->copyValueIds(firstPosition + i);
      if (store->loggingEnabled())
        io::Logger::getInstance().logValue(_txContext.tid, store->getName(), firstPosition + i, &vids);
#endif
    }
  } else {
    for (size_t i = 0; i < rowCount; ++i) {
      store->copyRowToDelta(_data, i, writeArea.first + i, _txContext.tid);

      // Update delta indices
      store->addRowToDeltaIndices(firstPosition + i);

      mods.insertPos(store, firstPosition + i);

#ifdef PERSISTENCY_BUFFEREDLOGGER
      std::vector<ValueId> vids = store.get()->copyValueIds(i);
      if (store->loggingEnabled())
        io::Logger::getInstance().logValue(_txContext.tid, store->getName(), firstPosition + i, &vids);
#endif
    }
  }

  auto rsp = getResponseTask();
  if (rsp != nullptr)
    rsp->incAffectedRows(rowCount);

  addResult(input.getTable(0));
}

void InsertScan::setInputData(const storage::atable_ptr_t& c) { _data = c; }

std::shared_ptr<PlanOperation> InsertScan::parse(const Json::Value& data) {
  auto result = std::make_shared<InsertScan>();

  if (data.isMember("data")) {
    result->_raw_data = functional::collect(data["data"], [](const Json::Value& v) {
      return functional::collect(v, [](const Json::Value& c) { return Json::Value(c); });
    });
  }

  /// ---> BEGIN HOTFIX
  else if (data.isMember("data_table")) {
    std::string tableName = data["data_table"].asString();
    auto sm = io::StorageManager::getInstance();
    if (sm->exists(tableName)) {
      result->setInputData(sm->getTable(tableName));
    } else {
      throw std::runtime_error("InsertScan: table not loaded");
    }
  }
  /// <--- END HOTFIX

  return result;
}
}
}
