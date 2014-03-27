#include "access/PosUpdateIncrementScan.h"

#include "access/system/QueryParser.h"
#include "access/system/ResponseTask.h"
#include "helper/checked_cast.h"
#include "io/TransactionManager.h"
#include "storage/AbstractTable.h"
#include "storage/PointerCalculator.h"
#include "storage/Store.h"
#include "access/json_converters.h"
#include "storage/meta_storage.h"
#include "io/TransactionError.h"

using namespace hyrise;
using namespace access;

namespace {
auto _ = QueryParser::registerPlanOperation<PosUpdateIncrementScan>("PosUpdateIncrementScan");
}


struct add_json_value_functor {

  typedef void value_type;

  storage::atable_ptr_t tab;
  size_t col;
  size_t row;
  Json::Value val;

  inline add_json_value_functor(storage::atable_ptr_t t) : tab(t) {}

  inline void set(size_t c, size_t r, Json::Value v) {
    col = c;
    row = r;
    val = v;
  }

  template <typename T>
  value_type operator()() {
    T oldVal = tab->getValue<T>(col, row);
    tab->setValue(col, row, oldVal + json_converter::convert<T>(val));
  }
};

PosUpdateIncrementScan::PosUpdateIncrementScan(std::string column, Json::Value offset)
    : _column(column), _offset(offset) {}

std::shared_ptr<PlanOperation> PosUpdateIncrementScan::parse(const Json::Value& data) {
  return std::make_shared<PosUpdateIncrementScan>(data["column"].asString(), data["offset"]);
}

void PosUpdateIncrementScan::executePlanOperation() {
  auto pc = checked_pointer_cast<const storage::PointerCalculator>(input.getTable(0));
  auto store =
      std::const_pointer_cast<storage::Store>(checked_pointer_cast<const storage::Store>(pc->getActualTable()));

  auto* positions = pc->getPositions();
  // Retrieve first row index of exclusive delta space
  auto delta_row = store->appendToDelta(positions->size()).first;

  auto& txmgr = tx::TransactionManager::getInstance();
  auto& modRecord = txmgr[_txContext.tid];

  add_json_value_functor fun(store->getDeltaTable());
  storage::type_switch<hyrise_basic_types> ts;

  std::size_t column_idx = store->numberOfColumn(_column);
  auto delta = store->getDeltaTable();
  auto main_size = store->getMainTable()->size();
  for (const auto& old_row : *positions) {
    if (store->markForDeletion(old_row, _txContext.tid) != tx::TX_CODE::TX_OK) {
      txmgr.rollbackTransaction(_txContext);
      throw tx::transaction_error("Aborted TX because TID of other TX found (Op: PosUpdateIncrementScan, Table: " +
                                  store->getName() + ")");
    }
    modRecord.deletePos(store, old_row);

    store->copyRowToDelta(store, old_row, delta_row, _txContext.tid);
    // Apply offset to original value in new row
    /*const auto& column_type = store->typeOfColumn(column_idx);
    delta->setValue<column_type>(column_idx, delta_row,
                                  delta->getValue<column_type>(column_idx, delta_row) +
    json_converter::convert<column_type>(_offset));
    */
    fun.set(column_idx, delta_row, _offset);
    ts(store->typeOfColumn(column_idx), fun);

    // Update delta indices
    store->addRowToDeltaIndices(main_size + delta_row);

    // add inserted pos to mod record. use absolute pos not pos in delta (!)
    modRecord.insertPos(store, main_size + delta_row);


#ifdef PERSISTENCY_BUFFEREDLOGGER
    if (store->loggingEnabled()) {
      std::vector<ValueId> vids = store->copyValueIds(main_size + delta_row);
      io::Logger::getInstance().logValue(_txContext.tid, store->getName(), main_size + delta_row, &vids);
      io::Logger::getInstance().logInvalidation(_txContext.tid, store->getName(), old_row);
    }
#endif

    ++delta_row;
  }

  if (auto rsp = getResponseTask()) {
    rsp->incAffectedRows(positions->size());
  }

  addResult(store);
}
