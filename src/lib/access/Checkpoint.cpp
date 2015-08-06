// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <access/Checkpoint.h>

#include <io/StorageManager.h>
#include <io/TableDump.h>
#include <io/logging.h>
#include "io/TransactionManager.h"

#include <storage/Store.h>
#include <helper/Settings.h>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<Checkpoint>("Checkpoint");
}

void Checkpoint::executePlanOperation() {
#ifdef PERSISTENCY_BUFFEREDLOGGER

  auto checkpoint_id = io::Logger::getInstance().startCheckpoint();
  if (checkpoint_id > 0) {
    std::string path = Settings::getInstance()->getCheckpointDir() + std::to_string(checkpoint_id) + "/";
    auto sm = io::StorageManager::getInstance();
    auto tablenames = sm->getTableNames();
    storage::SimpleTableDump delta_dumplor(path);
    storage::SimpleTableDump main_dumplor(Settings::getInstance()->getTableDumpDir());

    // go through all tables and save the size, so that we have a "most consistent snapshot" as possible.
    // e.g. we do not have too many updates coming in between the first dumped table and the last.
    for (auto tablename : tablenames) {
      auto store = std::dynamic_pointer_cast<storage::Store>(sm->getTable(tablename));
      if (store) {
        store->prepareCheckpoint();
      }
    }

    for (auto tablename : tablenames) {
      std::shared_ptr<storage::AbstractTable> a__table = sm->getTable(tablename);
      auto store = std::dynamic_pointer_cast<storage::Store>(a__table);

      // we only work on stores
      if (store) {
        if (_withMain) {
          main_dumplor.dump(tablename, a__table);
        }
        delta_dumplor.dumpDelta(tablename, a__table);
        delta_dumplor.dumpCidVectors(tablename, a__table);
      }
    }

    io::Logger::getInstance().endCheckpoint();
  }
#endif
}

std::shared_ptr<PlanOperation> Checkpoint::parse(const Json::Value& data) {
  auto op = std::make_shared<Checkpoint>();
  if (!data.isMember("withMain")) {
    op->setWithMain(false);
  } else {
    op->setWithMain(data["withMain"].asBool());
  }
  return op;
}

const std::string Checkpoint::vname() { return "Checkpoint"; }
}
}
