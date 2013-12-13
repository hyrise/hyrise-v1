// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "testing/TableEqualityTest.h"

#include "helper.h"

#include <access.h>
#include <algorithm>

#include <io/shortcuts.h>
#include <storage/Store.h>
#include <storage/PointerCalculator.h>
#include <storage/TableBuilder.h>
#include <helper/types.h>
#include <io/TransactionManager.h>

#include <testing/TableEqualityTest.h>

namespace hyrise { namespace access {

using storage::TableBuilder;

class TransactionTests : public AccessTest {

public:

  storage::c_atable_ptr_t data;
  storage::store_ptr_t linxxxs;

  storage::atable_ptr_t one_row;
  storage::atable_ptr_t second_row;

  void SetUp() {

    hyrise::tx::TransactionManager::getInstance().reset();

    TableBuilder::param_list list;
    list.append().set_type("INTEGER").set_name("a");
    list.append().set_type("FLOAT").set_name("b");
    list.append().set_type("STRING").set_name("c");

    list.appendGroup(1).appendGroup(1).appendGroup(1);

    TableBuilder::param_list list2;
    list2.append().set_type("INTEGER").set_name("col_0");
    list2.append().set_type("INTEGER").set_name("col_1");

    one_row = TableBuilder::build(list2);
    one_row->resize(1);
    one_row->setValue<hyrise_int_t>(0,0, 99);
    one_row->setValue<hyrise_int_t>(1,0, 999);

    second_row = TableBuilder::build(list2);
    second_row->resize(1);
    second_row->setValue<hyrise_int_t>(0,0, 22);
    second_row->setValue<hyrise_int_t>(1,0, 222);

    // Convert to store
    data = std::make_shared<storage::Store>(storage::TableBuilder::build(list));
    linxxxs = std::dynamic_pointer_cast<storage::Store>(io::Loader::shortcuts::load("test/lin_xxxs.tbl"));
  }


};


TEST_F(TransactionTests, read_own_writes) {

  auto writeCtx =hyrise::tx::TransactionManager::getInstance().buildContext();

  auto& mod = hyrise::tx::TransactionManager::getInstance()[writeCtx.tid];

  ASSERT_EQ(0u, mod.inserted.size());
  size_t before = linxxxs->size();
  // Add One read all
  InsertScan is;
  is.setTXContext(writeCtx);
  is.addInput(linxxxs);
  is.setInputData(one_row);
  is.execute();

  ASSERT_EQ(1u, mod.inserted.size());
  ASSERT_EQ(1u, mod.inserted[linxxxs].size());


  ProjectionScan ps;
  ps.addField(0);
  ps.setTXContext(writeCtx);
  ps.addInput(is.getResultTable());
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(writeCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto r1 = vp.getResultTable();
  ASSERT_EQ(before + 1, r1->size());
}

TEST_F(TransactionTests, read_only_commited) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  auto readCtx = tx::TransactionManager::getInstance().buildContext();

  size_t before = linxxxs->size();
  // Add One read all
  InsertScan is;
  is.setTXContext(writeCtx);
  is.addInput(linxxxs);
  is.setInputData(one_row);
  is.execute();

  auto& mod = tx::TransactionManager::getInstance()[readCtx.tid];
  ASSERT_EQ(0u, mod.inserted.size());

  ProjectionScan ps;
  ps.addField(0);
  ps.setTXContext(readCtx);
  ps.addInput(linxxxs);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(readCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();
  ASSERT_EQ(0u, mod.inserted.size());

  auto r1 = vp.getResultTable();
  ASSERT_EQ(before, r1->size());
}

TEST_F(TransactionTests, concurrent_writer) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  auto writeCtx2 = tx::TransactionManager::getInstance().buildContext();

  auto& mod = tx::TransactionManager::getInstance()[writeCtx.tid];
  ASSERT_EQ(0u, mod.inserted.size());


  size_t before = linxxxs->size();
  // Add One read all
  InsertScan is;
  is.setTXContext(writeCtx);
  is.addInput(linxxxs);
  is.setInputData(one_row);
  is.execute();


  ASSERT_EQ(1u, mod.inserted.size());
  ASSERT_EQ(1u, mod.inserted[linxxxs].size());

  // Add One read all
  InsertScan is2;
  is2.setTXContext(writeCtx2);
  is2.addInput(linxxxs);
  is2.setInputData(second_row);
  is2.execute();

  ASSERT_EQ(1u, mod.inserted[linxxxs].size());

  ASSERT_EQ(before + 2, linxxxs->size());

  // Commit the changes from the first TX and check that we still dont see it for the second
  Commit c;
  c.addInput(linxxxs);
  c.setTXContext(writeCtx);
  c.execute();

  ProjectionScan ps;
  ps.addInput(linxxxs);
  ps.setTXContext(writeCtx2);
  ps.addField(0);
  ps.addField(1);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(writeCtx2);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto res = vp.getResultTable();

  ASSERT_EQ(before + 1, res->size());
  ASSERT_EQ(222, res->getValue<hyrise_int_t>(1,res->size()-1));
}

TEST_F(TransactionTests, delete_op) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  size_t before = linxxxs->size();

  // Add One read all
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({0}));
  ASSERT_EQ(1u, pc->size());

  DeleteOp del;
  del.setTXContext(writeCtx);
  del.addInput(pc);
  del.execute();

  auto& mod = tx::TransactionManager::getInstance()[writeCtx.tid];
  ASSERT_EQ(1u, mod.deleted.size());

  // Commit the changes from the first TX and check that we still dont see it for the second
  Commit c;
  c.addInput(linxxxs);
  c.setTXContext(writeCtx);
  c.execute();

  auto readCtx = tx::TransactionManager::getInstance().buildContext();
  ProjectionScan ps;
  ps.addInput(linxxxs);
  ps.setTXContext(readCtx);
  ps.addField(0);
  ps.addField(1);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(readCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto res = vp.getResultTable();

  ASSERT_EQ(before - 1, res->size());
}


TEST_F(TransactionTests, read_your_own_deletes) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  size_t before = linxxxs->size();

  // Add One read all
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({0}));
  ASSERT_EQ(1u, pc->size());

  DeleteOp del;
  del.setTXContext(writeCtx);
  del.addInput(pc);
  del.execute();

  auto& mod = tx::TransactionManager::getInstance()[writeCtx.tid];
  ASSERT_EQ(1u, mod.deleted.size());

  ProjectionScan ps;
  ps.addInput(linxxxs);
  ps.setTXContext(writeCtx);
  ps.addField(0);
  ps.addField(1);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(writeCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto res = vp.getResultTable();
  ASSERT_EQ(before-1, res->size()) << "We expect not to see the row we deleted earlier";
}

TEST_F(TransactionTests, read_your_own_inserted_and_deleted) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();

  InsertScan is;
  is.setTXContext(writeCtx);
  is.addInput(linxxxs);
  is.setInputData(one_row);
  is.execute();

  size_t before = linxxxs->size();

  // Add One read all
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({before-1}));
  ASSERT_EQ(1u, pc->size());

  DeleteOp del;
  del.setTXContext(writeCtx);
  del.addInput(pc);
  del.execute();

  ProjectionScan ps;
  ps.addInput(linxxxs);
  ps.setTXContext(writeCtx);
  ps.addField(0);
  ps.addField(1);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(writeCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto res = vp.getResultTable();
  ASSERT_EQ(before-1, res->size()) << "We expect not to see the row we inserted and deleted earlier";
}


TEST_F(TransactionTests, delete_op_and_concurrent_read) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  size_t before = linxxxs->size();

  // Add One read all
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({0}));
  ASSERT_EQ(1u, pc->size());

  DeleteOp del;
  del.setTXContext(writeCtx);
  del.addInput(pc);
  del.execute();

  auto& mod = hyrise::tx::TransactionManager::getInstance()[writeCtx.tid];
  ASSERT_EQ(1u, mod.deleted.size());

  auto readCtx = tx::TransactionManager::getInstance().buildContext();
  ProjectionScan ps;
  ps.addInput(linxxxs);
  ps.setTXContext(readCtx);
  ps.addField(0);
  ps.addField(1);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(readCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto res = vp.getResultTable();

  ASSERT_EQ(before , res->size());
}

TEST_F(TransactionTests, delete_op_with_commit_and_concurrent_read) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  size_t before = linxxxs->size();

  // Delete positiotn 0
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({0}));
  ASSERT_EQ(1u, pc->size());

  DeleteOp del;
  del.setTXContext(writeCtx);
  del.addInput(pc);
  del.execute();


  // Acquire the TX Lock
  auto& txmgr = hyrise::tx::TransactionManager::getInstance();
  writeCtx.cid = txmgr.prepareCommit();

  // write commit id to simulate transaction in the process of committing
  linxxxs->commitPositions(*(pc->getPositions()), writeCtx.cid, false);



  auto& mod = tx::TransactionManager::getInstance()[writeCtx.tid];
  ASSERT_EQ(1u, mod.deleted.size());

  // read all
  auto readCtx = tx::TransactionManager::getInstance().buildContext();
  ProjectionScan ps;
  ps.addInput(linxxxs);
  ps.setTXContext(readCtx);
  ps.addField(0);
  ps.addField(1);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(readCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto res = vp.getResultTable();

  txmgr.commit(writeCtx.tid);

  ASSERT_EQ(before , res->size());
}

TEST_F(TransactionTests, delete_op_and_merge) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  size_t before = linxxxs->size();

  // Add One read all
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({0}));
  ASSERT_EQ(1u, pc->size());

  DeleteOp del;
  del.setTXContext(writeCtx);
  del.addInput(pc);
  del.execute();

  auto& mod = hyrise::tx::TransactionManager::getInstance()[writeCtx.tid];
  ASSERT_EQ(1u, mod.deleted.size());

  // Commit the changes from the first TX and check that we still dont see it for the second
  Commit c;
  c.addInput(linxxxs);
  c.setTXContext(writeCtx);
  c.execute();

  //No merge the table
  MergeStore mt;
  mt.addInput(linxxxs);
  mt.execute();

  auto result = mt.getResultTable();


  // Check after merge
  auto readCtx = tx::TransactionManager::getInstance().buildContext();
  ProjectionScan ps;
  ps.addInput(result);
  ps.setTXContext(readCtx);
  ps.addField(0);
  ps.addField(1);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(readCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto res = vp.getResultTable();

  ASSERT_EQ(before - 1, res->size());
}


TEST_F(TransactionTests, update_and_read_values) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  auto& mod = tx::TransactionManager::getInstance()[writeCtx.tid];

  ASSERT_EQ(0u, mod.inserted.size());
  size_t before = linxxxs->size();

  // create PC to simulate position
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({0,4}));

  PosUpdateScan is;
  is.setTXContext(writeCtx);
  is.addInput(pc);

  Json::Value v;
  v["col_1"] = 99;
  is.setRawData(v);
  is.execute();

  ASSERT_EQ(1u, mod.inserted.size());
  ASSERT_EQ(2u, mod.inserted[linxxxs].size());

  ASSERT_EQ(1u, mod.deleted.size());
  ASSERT_EQ(2u, mod.deleted[linxxxs].size());

  ProjectionScan ps;
  ps.addField(0);
  ps.setTXContext(writeCtx);
  ps.addInput(is.getResultTable());
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(writeCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto r1 = vp.getResultTable();
  ASSERT_EQ(before, r1->size());
}

TEST_F(TransactionTests, update_and_merge) {

  auto writeCtx = tx::TransactionManager::getInstance().buildContext();
  auto& mod = tx::TransactionManager::getInstance()[writeCtx.tid];

  ASSERT_EQ(0u, mod.inserted.size());
  size_t before = linxxxs->size();

  // create PC to simulate position
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({0,4}));

  PosUpdateScan is;
  is.setTXContext(writeCtx);
  is.addInput(pc);

  Json::Value v;
  v["col_1"] = 99;
  is.setRawData(v);
  is.execute();

  ASSERT_EQ(1u, mod.inserted.size());
  ASSERT_EQ(2u, mod.inserted[linxxxs].size());

  ASSERT_EQ(1u, mod.deleted.size());
  ASSERT_EQ(2u, mod.deleted[linxxxs].size());

  Commit c;
  c.addInput(linxxxs);
  c.setTXContext(writeCtx);
  c.execute();

  writeCtx =hyrise::tx::TransactionManager::getInstance().buildContext();

  //No merge the table
  MergeStore mt;
  mt.addInput(linxxxs);
  mt.setTXContext(writeCtx);
  mt.execute();

  auto result = mt.getResultTable();
  const auto& ref =  io::Loader::shortcuts::load("test/reference/lin_xxxs_update.tbl");
  EXPECT_RELATION_EQ(ref, result);


  ProjectionScan ps;
  ps.addField(0);
  ps.addField(1);
  ps.setTXContext(writeCtx);
  ps.addInput(result);
  ps.execute();

  ValidatePositions vp;
  vp.setTXContext(writeCtx);
  vp.addInput(ps.getResultTable());
  vp.execute();

  auto r1 = vp.getResultTable();
  ASSERT_EQ(before, r1->size());
  ASSERT_EQ(99, r1->getValue<hyrise_int_t>(1,3));
  ASSERT_EQ(99, r1->getValue<hyrise_int_t>(1,4));
}

TEST_F(TransactionTests, delete_rollback) {
  auto writeCtx = tx::TransactionManager::beginTransaction();
  auto pc = storage::PointerCalculator::create(linxxxs, new pos_list_t({0}));
  ASSERT_EQ(tx::START_TID, linxxxs->tid(0));

  DeleteOp del;
  del.setTXContext(writeCtx);
  del.addInput(pc);
  del.execute();

  tx::TransactionManager::rollbackTransaction(writeCtx);
  ASSERT_EQ(tx::START_TID, linxxxs->tid(0));
}

}}
