#include "access/insertonly.h"

#include "access/insertonly_merger.h"
#include "access/insertonly_valid.h"
#include "helper/types.h"
#include "io/loaders.h"
#include "io/ValidityTableGeneration.h"
#include "storage/AbstractTable.h"
#include "storage/PointerCalculator.h"
#include "storage/PointerCalculatorFactory.h"
#include "storage/SimpleStore.h"
#include "storage/LogarithmicMergeStrategy.h"

namespace hyrise {
namespace insertonly {

AbstractTable::SharedTablePtr construct(std::string filename, const tx::transaction_id_t& from_tid) {
  auto ret = Loader::load(Loader::params()
                      .setInsertOnly({true, from_tid})
                      .setHeader(CSVHeader(filename))
                      .setInput(CSVInput(filename)));
  assureInsertOnly(ret);
  return ret;
}

bool columnExists(const storage::c_atable_ptr_t& table,
                  const std::string& col) {
  try {
    table->numberOfColumn(col);
  } catch (const MissingColumnException&) {
    return false;
  }
  return true;
}

storage::c_simplestore_ptr_t isInsertOnly(const storage::c_atable_ptr_t& table) {
  // TODO: Check for table being writeable in both columns in all
  // mains and delta
  auto store = std::dynamic_pointer_cast<const storage::SimpleStore>(table);
  if (store && columnExists(store, VALID_TO_COL_ID) && columnExists(store, VALID_FROM_COL_ID)) {
    return store;
  }
  return nullptr;
}

storage::simplestore_ptr_t assureInsertOnly(const storage::c_atable_ptr_t& table) {
  const auto& tbl = std::const_pointer_cast<hyrise::storage::SimpleStore>(isInsertOnly(table));
  if (tbl) return tbl;
  throw std::runtime_error("Passed table was not InsertOnly");
}

void insertRows(const storage::simplestore_ptr_t& store,
                const storage::c_atable_ptr_t& rows,
                const tx::transaction_id_t& tid) {
  auto delta = store->getDelta();
  std::vector<storage::atable_ptr_t> tables
      {std::const_pointer_cast<AbstractTable>(rows), validityTableForTransaction(rows->size(), tid, VISIBLE)};
  // TODO: Needs View Table
  auto table = std::make_shared<MutableVerticalTable>(tables, rows->size());
  delta->appendRows(table);
}

void deleteRows(const storage::simplestore_ptr_t& store,
                const storage::pos_list_t& positions,
                const tx::transaction_id_t& tid) {
  const auto& valid_to_position = store->numberOfColumn(VALID_TO_COL_ID);
  for (const auto& position: positions) {
    store->setValue<hyrise_int_t>(valid_to_position, position, tid);
  }
}

template <typename T>
void invalidate(const T& store, const size_t& valid_to_position, const storage::pos_list_t& positions, const tx::transaction_id_t& tid) {
  for (const auto& position: positions) {
    store->template setValue<hyrise_int_t>(valid_to_position, position, tid);
  }
}

void deleteRows(const storage::simplestore_ptr_t& store,
                const storage::pos_list_t& positions_main,
                const storage::pos_list_t& positions_delta,
                const tx::transaction_id_t& tid) {
  const auto& valid_to_position = store->numberOfColumn(VALID_TO_COL_ID);
  invalidate(store->getMain(), valid_to_position, positions_main, tid);
  invalidate(store->getDelta(), valid_to_position, positions_delta, tid);
}

void updateRows(const storage::simplestore_ptr_t& store,
                const storage::c_atable_ptr_t& rows,
                const pos_list_t& invalidate_positions_main,
                const pos_list_t& invalidate_positions_delta,
                const tx::transaction_id_t& tid) {
  deleteRows(store, invalidate_positions_main, invalidate_positions_delta, tid);
  insertRows(store, rows, tid);
}

void updateRows(const storage::simplestore_ptr_t& store,
                const storage::c_atable_ptr_t& rows,
                const pos_list_t& update_positions,
                const tx::transaction_id_t& tid) {
  deleteRows(store, update_positions, tid);
  insertRows(store, rows, tid);
}


storage::atable_ptr_t filterValid(const storage::simplestore_ptr_t& store,
                                  const tx::transaction_id_t& tid) {
  return validPointerCalculator(store, tid);
}

storage::atable_ptr_t validPositionsDelta(const storage::simplestore_ptr_t& store,
                                          const tx::transaction_id_t& tid) {
  return validPointerCalculator(store->getDelta(), tid);
}

storage::atable_ptr_t validPositionsMain(const storage::simplestore_ptr_t& store,
                                         const tx::transaction_id_t& tid) {
  return validPointerCalculator(store->getMain(), tid);
}

storage::atable_ptr_t merge(const storage::simplestore_ptr_t& store,
                            const tx::transaction_id_t& tid) {
  store->mergeWith(std::unique_ptr<TableMerger>(new TableMerger(new LogarithmicMergeStrategy(0), new DiscardingMerger(tid))));
  return store;
}

}}
