// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PosUpdateScan.h"
#include <access/system/ResponseTask.h>

#include "json_converters.h"

#include "helper/vector_helpers.h"
#include "helper/checked_cast.h"

#include "io/TransactionManager.h"

#include "storage/Store.h"
#include "storage/PointerCalculator.h"
#include "storage/AbstractTable.h"
#include "storage/meta_storage.h"


namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<PosUpdateScan>("PosUpdateScan");
}



PosUpdateScan::~PosUpdateScan() {
}


void PosUpdateScan::executePlanOperation() {
  auto c_pc = checked_pointer_cast<const PointerCalculator>(input.getTable(0));
  auto c_store = checked_pointer_cast<const storage::Store>(c_pc->getActualTable());

  // Cast the constness away
  auto store = std::const_pointer_cast<storage::Store>(c_store);

  // Get the current maximum size
  const auto& beforSize = store->size();

  // Get the offset for inserts into the delta and the size of the delta that
  // we need to increase by the positions we are inserting
  auto writeArea = store->appendToDelta(c_pc->getPositions()->size());

  // Get the modification record for the current transaction
  auto& txmgr = tx::TransactionManager::getInstance();
  auto& modRecord = txmgr[_txContext.tid];

  // Functor we use for updating the data
  set_json_value_functor fun(store->getDeltaTable());
  storage::type_switch<hyrise_basic_types> ts;

  size_t counter = 0;
  for(const auto& p : *(c_pc->getPositions())) {
    // First delete the old record
    bool deleteOk = store->markForDeletion(p, _txContext.tid) == hyrise::tx::TX_CODE::TX_OK;
    if(!deleteOk) {
      txmgr.abort();
      throw std::runtime_error("Aborted TX because TID of other TX found");
    }
    modRecord.deletePos(store, p);
    //store->setTid(p, _txContext.tid);

    // Copy the old row from the main
    store->copyRowToDelta(store, p, writeArea.first+counter, _txContext.tid);
    // Update all the necessary values
    for(const auto& kv : _raw_data) {
      const auto& fld = store->numberOfColumn(kv.first);
      fun.set(fld, writeArea.first+counter, *(kv.second.get()));
      ts(store->typeOfColumn(fld), fun);
    }

    // Insert the new one
    modRecord.insertPos(store, beforSize+counter);
    ++counter;
  }

  // Update affected rows
  auto rsp = getResponseTask();
  if (rsp != nullptr)
    rsp->incAffectedRows(counter);

  addResult(c_store);
}

void PosUpdateScan::setRawData(const rapidjson::Value& d) {
  for(const auto& m : d.getMemberNames()) {
    auto tmp = std::unique_ptr<rapidjson::Document>(new rapidjson::Document());
    d[m].Accept(*tmp);
    _ram_data.insert(std::pair<std::string, std::unique_ptr<rapidjson::Document>>(m, std::move(tmp)));
    //_raw_data[m] = stmp;
  }
}

std::shared_ptr<PlanOperation> PosUpdateScan::parse(const rapidjson::Value &data) {
  auto result = std::make_shared<PosUpdateScan>();

  if (data.isMember("data")) {
    result->setRawData(data["data"]);
  }
  return result;
}

}
}
