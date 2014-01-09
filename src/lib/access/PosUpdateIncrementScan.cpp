#include "access/PosUpdateIncrementScan.h"

#include "access/system/QueryParser.h"
#include "access/system/ResponseTask.h"
#include "helper/checked_cast.h"
#include "io/TransactionManager.h"
#include "storage/AbstractTable.h"
#include "storage/PointerCalculator.h"
#include "storage/Store.h"

using namespace hyrise;
using namespace access;

namespace {
auto _ = QueryParser::registerPlanOperation<PosUpdateIncrementScan>("PosUpdateIncrementScan");
}

PosUpdateIncrementScan::PosUpdateIncrementScan(std::string column, hyrise_int_t offset) : _column(column), _offset(offset) {} 

std::shared_ptr<PlanOperation> PosUpdateIncrementScan::parse(Json::Value& data) {
  return std::make_shared<PosUpdateIncrementScan>(data["column"].asString(), data["offset"].asInt());
}

void PosUpdateIncrementScan::executePlanOperation() {
  auto pc = checked_pointer_cast<const PointerCalculator>(input.getTable(0));
  auto store = std::const_pointer_cast<storage::Store>(
      checked_pointer_cast<const storage::Store>(pc->getActualTable()));

  auto* positions = pc->getPositions();
  // Retrieve first row index of exclusive delta space
  auto delta_row = store->appendToDelta(positions->size()).first;

  auto& txmgr = tx::TransactionManager::getInstance();
  auto& modRecord = txmgr[_txContext.tid];

  std::size_t column_idx = store->numberOfColumn(_column);
  auto delta = store->getDeltaTable();
  for (const auto& old_row: *positions) {
    if (store->markForDeletion(old_row, _txContext.tid) != tx::TX_CODE::TX_OK) {
      txmgr.abort();
      throw std::runtime_error("Aborted TX because TID of other TX found");
    }
    modRecord.deletePos(store, old_row);
    
    store->copyRowToDelta(store, old_row, delta_row, _txContext.tid);
    // Apply offset to original value in new row
    delta->setValue<hyrise_int_t>(column_idx, delta_row,
                                  delta->getValue<hyrise_int_t>(column_idx, delta_row) + _offset);
    modRecord.insertPos(store, delta_row);
    ++delta_row;
  }

  if (auto rsp = getResponseTask()) {
    rsp->incAffectedRows(positions->size());
  }

  addResult(store);
}
