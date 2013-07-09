// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
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
#include "storage/AbstractMergeStrategy.h"

namespace hyrise {
namespace insertonly {

hyrise::storage::atable_ptr_t construct(std::string filename, const tx::TXContext& ctx) {
  auto ret = Loader::load(Loader::params()
                      .setInsertOnly({true, ctx.tid})
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
                const tx::TXContext& ctx) {
  auto delta = store->getDelta();
  std::vector<storage::atable_ptr_t> tables
      {std::const_pointer_cast<AbstractTable>(rows), validityTableForTransaction(rows->size(), ctx.tid, VISIBLE)};
  // TODO: Needs View Table
  auto table = std::make_shared<MutableVerticalTable>(tables, rows->size());
  delta->appendRows(table);
}

void deleteRows(const storage::simplestore_ptr_t& store,
                const storage::pos_list_t& positions,
                const tx::TXContext& ctx) {
  const auto& valid_to_position = store->numberOfColumn(VALID_TO_COL_ID);
  for (const auto& position: positions) {
    store->setValue<hyrise_int_t>(valid_to_position, position, ctx.tid);
  }
}

template <typename T>
void invalidate(const T& store, const size_t& valid_to_position, const storage::pos_list_t& positions, const tx::TXContext& ctx) {
  for (const auto& position: positions) {
    store->template setValue<hyrise_int_t>(valid_to_position, position, ctx.tid);
  }
}

void deleteRows(const storage::simplestore_ptr_t& store,
                const storage::pos_list_t& positions_main,
                const storage::pos_list_t& positions_delta,
                const tx::TXContext& ctx) {
  const auto& valid_to_position = store->numberOfColumn(VALID_TO_COL_ID);
  invalidate(store->getMain(), valid_to_position, positions_main, ctx);
  invalidate(store->getDelta(), valid_to_position, positions_delta, ctx);
}

void updateRows(const storage::simplestore_ptr_t& store,
                const storage::c_atable_ptr_t& rows,
                const pos_list_t& invalidate_positions_main,
                const pos_list_t& invalidate_positions_delta,
                const tx::TXContext& ctx) {
  deleteRows(store, invalidate_positions_main, invalidate_positions_delta, ctx);
  insertRows(store, rows, ctx);
}

void updateRows(const storage::simplestore_ptr_t& store,
                const storage::c_atable_ptr_t& rows,
                const pos_list_t& update_positions,
                const tx::TXContext& ctx) {
  deleteRows(store, update_positions, ctx);
  insertRows(store, rows, ctx);
}


storage::atable_ptr_t filterValid(const storage::simplestore_ptr_t& store,
                                  const tx::TXContext& ctx) {
  return validPointerCalculator(store, ctx.tid);
}

storage::atable_ptr_t validPositionsDelta(const storage::simplestore_ptr_t& store,
                                          const tx::TXContext& ctx) {
  return validPointerCalculator(store->getDelta(), ctx.tid);
}

storage::atable_ptr_t validPositionsMain(const storage::simplestore_ptr_t& store,
                                         const tx::TXContext& ctx) {
  return validPointerCalculator(store->getMain(), ctx.tid);
}

storage::atable_ptr_t merge(const storage::simplestore_ptr_t& store,
                            const tx::TXContext& ctx) {
  store->mergeWith(std::unique_ptr<TableMerger>(new TableMerger(new DefaultMergeStrategy(), new DiscardingMerger(ctx.tid))));
  return store;
}

}}
