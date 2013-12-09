// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <access.h>
#include <storage.h>
#include <io.h>

//JoinScan Benchmark similar to TPC-C Implementation of Stock-Level Transaction
//See TPC-C Reference Chapter A.5

namespace hyrise {
namespace access {

class JoinScanBase : public ::testing::Benchmark {

 protected:

  io::StorageManager *sm;
  std::shared_ptr<JoinScan> js;
  storage::atable_ptr_t t1;
  storage::atable_ptr_t t2;

 public:
  void BenchmarkSetUp() {
    sm = io::StorageManager::getInstance();

    auto js = std::make_shared<JoinScan>(JoinType::EQUI);
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

  hyrise::storage::atable_ptr_t result = js->execute();
  }*/

} } // namespace hyrise::access

