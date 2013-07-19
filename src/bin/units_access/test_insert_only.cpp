// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "io/shortcuts.h"
#include "access/insertonly.h"
#include "io/TransactionManager.h"
#include "storage/SimpleStore.h"

namespace hyrise {
namespace insertonly {

static const tx::TXContext load_txid;

class InsertOnly : public Test {
 protected:
  tx::TransactionManager& tm;
  storage::simplestore_ptr_t store;
  storage::atable_ptr_t row;

  storage::simplestore_ptr_t load() {
    auto table = construct("test/tables/employees.tbl", load_txid);
    return assureInsertOnly(table);
  }

  InsertOnly() : tm(tx::TransactionManager::getInstance()) {}
  virtual void SetUp() {
    store = load();
    row = Loader::shortcuts::load("test/tables/employees_new_row.tbl");
  }
};

TEST_F(InsertOnly, construct) {
  EXPECT_NE(isInsertOnly(store), nullptr) << "constructed table has to be insertonly";
  EXPECT_EQ(filterValid(store, load_txid)->size(), store->size());
}

TEST_F(InsertOnly, insert) {
  const size_t store_sz = store->size(),
      row_sz = row->size();

  tx::TXContext insert_txid(2,0);
  insertRows(store, row, insert_txid);
  EXPECT_EQ(store->size(), store_sz + row_sz);
  EXPECT_EQ(filterValid(store, insert_txid)->size(), store_sz + row_sz);
}

TEST_F(InsertOnly, delete) {
  auto store_sz = store->size();
  tx::TXContext delete_txid(2,0);
  deleteRows(store, {0}, delete_txid);
  EXPECT_EQ(store->size(), store_sz)
      << "Store size does not change when deleting rows";
  EXPECT_EQ(filterValid(store, delete_txid)->size(), store_sz - 1)
      << "A valid view decreases in size after deletion";
}

TEST_F(InsertOnly, delete_previously_added) {
  auto store_sz = store->size();
  tx::TXContext update_txid(2,0);
  tx::TXContext delete_txid(3,0);
  updateRows(store, row, {0}, update_txid);
  deleteRows(store, {store->size() - 1}, delete_txid);
  EXPECT_EQ(filterValid(store, delete_txid)->size(), store_sz-1)
      << "ValidView should be smaller than initial size";
}

TEST_F(InsertOnly, update) {
  const size_t store_sz = store->size();
  tx::TXContext update_txid(2,0);
  updateRows(store, row, {0}, update_txid);
  EXPECT_EQ(store->size(), store_sz + 1);
  EXPECT_EQ(filterValid(store, update_txid)->size(), store_sz)
      << "A valid view does not change in size";
}

TEST_F(InsertOnly, update_merge) {
  const size_t store_sz = store->size();
  tx::TXContext update_txid(2,0);
  updateRows(store, row, {0}, update_txid);
  auto r = merge(store, tx::TXContext(update_txid.tid+1,0));
  EXPECT_EQ(store_sz, r->size())
      << "Updating a single row should lead to unchanged store size";
}

TEST_F(InsertOnly, insert_merge) {
  const size_t store_sz = store->size();
  tx::TXContext update_txid(2,0);
  insertRows(store, row, update_txid);
  auto r = merge(store, update_txid);
  EXPECT_EQ(store_sz + 1, r->size())
      << "Inserting a row should lead to increased store size";
}

TEST_F(InsertOnly, delete_merge) {
  const size_t store_sz = store->size();
  tx::TXContext delete_txid(2,0);
  deleteRows(store, {0, 2}, delete_txid);
  auto r = merge(store, delete_txid);
  EXPECT_EQ(store_sz-2, r->size())
      << "Deleting two rows should lead to decreased store size";
}

}
}
