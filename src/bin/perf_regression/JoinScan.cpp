// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <access.h>
#include <storage.h>
#include <io.h>

//JoinScan Benchmark similar to TPC-C Implementation of Stock-Level Transaction
//See TPC-C Reference Chapter A.5

class JoinScanBase : public ::testing::Benchmark {

 protected:

  StorageManager *sm;
  std::shared_ptr<hyrise::access::JoinScan> js;
  AbstractTable::SharedTablePtr t1;
  AbstractTable::SharedTablePtr t2;

 public:
  void BenchmarkSetUp() {
    sm = StorageManager::getInstance();

    auto js = std::make_shared<hyrise::access::JoinScan>(hyrise::access::JoinType::EQUI);
    js->setEvent("NO_PAPI");

    t1 = sm->getTable("order_line");
    t2 = sm->getTable("stock");

    js->addInput(t1);
    js->addInput(t2);
  }

  void BenchmarkTearDown() {
  }

  JoinScanBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};

/*BENCHMARK_F(JoinScanBase, stock_level_equi_join)
  {
  js->addJoinClause<int>(0, 4, 1, 0);

  AbstractTable::SharedTablePtr result = js->execute();
  }*/
