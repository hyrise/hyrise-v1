// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <helper/types.h>
#include <access.h>
#include <storage.h>
#include <io.h>

namespace hyrise {
namespace access {

//HashJoinScan Benchmark similar to TPC-C Implementation of Stock-Level Transaction
//See TPC-C Reference Chapter A.5

class HashJoinBase : public ::testing::Benchmark {

 protected:

  io::StorageManager *sm;

  HashBuild *hb;
  HashJoinProbe *hjp;
  storage::c_atable_ptr_t t1;
  storage::c_atable_ptr_t t2;

 public:
  void BenchmarkSetUp() {
    hb = new HashBuild();
    hjp = new HashJoinProbe;
    hjp->setEvent("NO_PAPI");
    hb->setKey("join");
    hb->setEvent("NO_PAPI");
    sm = io::StorageManager::getInstance();

    t1 = sm->getTable("stock");
    t2 = sm->getTable("order_line");

    hb->addInput(t1);
    hb->addField(0);
    hjp->addInput(t2);
    hjp->addField(4);

  }

  void BenchmarkTearDown() {

  }

  HashJoinBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};

BENCHMARK_F(HashJoinBase, stock_level_hash_join) {
  auto hashedColumn = hb->execute()->getResultHashTable();
  assert(hashedColumn != nullptr);
  hjp->addInput(hashedColumn);
  auto result = hjp->execute()->getResultTable();
}

BENCHMARK_F(HashJoinBase, stock_level_hash_join_mat) {
  auto hashedColumn = hb->execute()->getResultHashTable();
  hjp->addInput(hashedColumn);
  auto result = hjp->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(false);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();

}

BENCHMARK_F(HashJoinBase, stock_level_hash_join_mat_memcpy) {
  auto hashedColumn = hb->execute()->getResultHashTable();
  hjp->addInput(hashedColumn);
  auto result = hjp->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(true);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();
}

}
}
