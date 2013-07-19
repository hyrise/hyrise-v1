// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PosUpdateScan.h"

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
  auto c_store = checked_pointer_cast<const Store>(c_pc->getActualTable());
  
  // Cast the constness away
  auto store = std::const_pointer_cast<Store>(c_store);

  // Get the current maximum size
  const auto& beforSize = store->size();

  // Get the offset for inserts into the delta and the size of the delta that
  // we need to increase by the positions we are inserting
  size_t deltaMax = store->getDeltaTable()->size();
  auto writeArea = store->resizeDelta(deltaMax + c_pc->getPositions()->size());

  // Get the modification record for the current transaction
  auto& modRecord = tx::TransactionManager::getInstance()[_txContext.tid];

  // Functor we use for updating the data
  set_json_value_functor fun(store->getDeltaTable());
  storage::type_switch<hyrise_basic_types> ts;

  size_t counter = 0;
  const auto& hidden = false;
  for(const auto& p : *(c_pc->getPositions())) {
    // First delete the old record
    modRecord.deletePos(store, p);
    store->setTid(p, _txContext.tid);

    // Copy the old row from the main
    store->copyRowToDelta(store, p, writeArea.first+counter, _txContext.tid, hidden);
    // Update all the necessary values
    for(const auto& kv : _raw_data) {
      const auto& fld = store->numberOfColumn(kv.first);
      fun.set(fld, writeArea.first+counter, kv.second);
      ts(store->typeOfColumn(fld), fun);
    }

    // Insert the new one
    modRecord.insertPos(store, beforSize+counter);
    ++counter;
  }

  addResult(c_store);
}

void PosUpdateScan::setRawData(Json::Value& d) {
  for(const auto& m : d.getMemberNames()) {
      _raw_data[m] = Json::Value(d[m]);
    }
}

std::shared_ptr<PlanOperation> PosUpdateScan::parse(Json::Value &data) {
  auto result = std::make_shared<PosUpdateScan>();

  if (data.isMember("data")) {
    result->setRawData(data["data"]);
  }
  return result;
}

}
}
