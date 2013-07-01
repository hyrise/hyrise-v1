// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <access.h>
#include <access/InsertScan.h>
#include <access/tx/Commit.h>
#include <storage.h>
#include <storage/TableBuilder.h>
#include <io.h>
#include <io/shortcuts.h>
#include <io/TransactionManager.h>

#include <helper/types.h>

//JoinScan Benchmark similar to TPC-C Implementation of Stock-Level Transaction
//See TPC-C Reference Chapter A.5

class TXBase : public ::testing::Benchmark {

 protected:

  hyrise::storage::store_ptr_t linxxxs;

  hyrise::storage::atable_ptr_t one_row;

 public:
  void BenchmarkSetUp() {

    TableBuilder::param_list list2;
    list2.append().set_type("INTEGER").set_name("col_0");
    list2.append().set_type("INTEGER").set_name("col_1");

    one_row = TableBuilder::build(list2);
    one_row->resize(1);
    one_row->setValue<hyrise_int_t>(0,0, 99);
    one_row->setValue<hyrise_int_t>(1,0, 999);

    linxxxs = std::dynamic_pointer_cast<Store>(Loader::shortcuts::load("test/lin_xxxs.tbl"));
  }

  void BenchmarkTearDown() {
  }

  TXBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};

BENCHMARK_F( TXBase, simple_commit) {
  for(size_t i=0; i < 10000; ++i) {
    auto&  txmgr = hyrise::tx::TransactionManager::getInstance();
    auto ctx = txmgr.buildContext();

    hyrise::access::InsertScan is;
    is.setTXContext(ctx);
    is.addInput(linxxxs);
    is.setInputData(one_row);
    is.execute();

    hyrise::access::Commit c;
    c.addInput(linxxxs);
    c.setTXContext(ctx);
    c.execute();    
  }
}

/*BENCHMARK_F(JoinScanBase, stock_level_equi_join)
  {
  js->addJoinClause<int>(0, 4, 1, 0);

  hyrise::storage::atable_ptr_t result = js->execute();
  }*/
