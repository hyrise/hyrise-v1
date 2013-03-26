// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <access.h>
#include <storage.h>
#include <io.h>

class ProjectionScanBase : public ::testing::Benchmark {

 protected:

  StorageManager *sm;
  hyrise::access::ProjectionScan *ps;
  AbstractTable::SharedTablePtr t;

 public:
  void BenchmarkSetUp() {
    ps = new hyrise::access::ProjectionScan();
    ps->setEvent("NO_PAPI");

    sm = StorageManager::getInstance();

    t = sm->getTable("district");

    ps->setProducesPositions(true);
    ps->addInput(t);
    ps->addField(0);
    ps->addField(1);
  }

  void BenchmarkTearDown() {
  }

  ProjectionScanBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};

BENCHMARK_F(ProjectionScanBase, project_new_order_tpcc_district) {
  //SELECT d_next_o_id, d_tax FROM district
  auto result = ps->execute()->getResultTable();
}

BENCHMARK_F(ProjectionScanBase, project_new_order_tpcc_district_mat) {
  //SELECT d_next_o_id, d_tax FROM district
  //materialized
  auto result = ps->execute()->getResultTable();

  hyrise::access::MaterializingScan *ms = new hyrise::access::MaterializingScan(false);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();
}

BENCHMARK_F(ProjectionScanBase, project_new_order_tpcc_district_mat_memcpy) {
  //SELECT d_next_o_idd_tax FROM district
  //materialized
  //memcpy

  auto result = ps->execute()->getResultTable();

  hyrise::access::MaterializingScan *ms = new hyrise::access::MaterializingScan(true);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();
}
