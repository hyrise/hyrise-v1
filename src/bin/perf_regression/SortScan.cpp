// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <access.h>
#include <storage.h>
#include <io.h>

namespace hyrise {
namespace access {

class SortScanBase : public ::testing::Benchmark {

 protected:

  io::StorageManager *sm;
  SortScan *sc;
  storage::c_atable_ptr_t t;

 public:
  void BenchmarkSetUp() {
    sm = io::StorageManager::getInstance();

    sc = new SortScan();
    sc->setEvent("NO_PAPI");

    t = sm->getTable("item");
    sc->addInput(t);
  }

  void BenchmarkTearDown() {

  }
  SortScanBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};

BENCHMARK_F(SortScanBase, simple_sort_scan_int) {
  sc->setSortField(1);
  auto result = sc->execute()->getResultTable();
}

BENCHMARK_F(SortScanBase, simple_sort_scan_float) {
  sc->setSortField(3);
  auto result = sc->execute()->getResultTable();
}

BENCHMARK_F(SortScanBase, simple_sort_scan_string) {
  sc->setSortField(4);
  auto result = sc->execute()->getResultTable();
}

BENCHMARK_F(SortScanBase, simple_sort_scan_sorted_int) {
  sc->setSortField(0);
  auto result = sc->execute()->getResultTable();
}

BENCHMARK_F(SortScanBase, simple_sort_scan_int_mat) {
  sc->setSortField(1);
  auto result = sc->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(false);
  ms->setEvent("NO_PAPI");

  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();

}

BENCHMARK_F(SortScanBase, simple_sort_scan_int_mat_memcpy) {
  sc->setSortField(1);
  auto result = sc->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(true);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();

}

BENCHMARK_F(SortScanBase, simple_sort_scan_string_mat) {
  sc->setSortField(4);
  auto result = sc->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(false);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();

}

BENCHMARK_F(SortScanBase, simple_sort_scan_string_mat_memcpy) {
  sc->setSortField(4);
  auto result = sc->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(true);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();

}

BENCHMARK_F(SortScanBase, simple_sort_scan_sorted_int_mat) {
  sc->setSortField(0);
  auto result = sc->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(false);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();
}

BENCHMARK_F(SortScanBase, simple_sort_scan_sorted_int_mat_memcpy) {
  sc->setSortField(0);
  auto result = sc->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(true);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  auto result_mat = ms->execute()->getResultTable();

}

}
}
