// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <access.h>
#include <storage.h>
#include <io.h>

namespace hyrise {
namespace access {

class UnionScanBase : public ::testing::Benchmark {

 protected:

  UnionScan *us;
  StorageManager *sm;
  storage::c_atable_ptr_t t1;
  storage::c_atable_ptr_t t2;

 public:
  void BenchmarkSetUp() {
    sm = StorageManager::getInstance();

    us = new UnionScan();
    us->setEvent("NO_PAPI");

    t1 = sm->getTable("district");
    t2 = sm->getTable("district");


    us->addInput(t1);
    us->addInput(t2);

    us->setProducesPositions(true);
  }

  void BenchmarkTearDown() {

  }

  UnionScanBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};

BENCHMARK_F(UnionScanBase, standard_union_with_pos) {
  auto result = us->execute()->getResultTable();
}

BENCHMARK_F(UnionScanBase, standard_union_mat) {
  auto result = us->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(false);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();

}

BENCHMARK_F(UnionScanBase, standard_union_mat_memcpy) {
  auto result = us->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(true);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();

}

}
}
