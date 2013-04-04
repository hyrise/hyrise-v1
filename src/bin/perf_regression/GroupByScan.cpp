// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <access.h>
#include <storage.h>
#include <io.h>

// GroupByScan as derived from TPC-C Delivery Transaction
// See TPC-C Reference Section A.4

class GroupByScanBase : public ::testing::Benchmark {

 protected:

  StorageManager *sm;
  std::shared_ptr<hyrise::access::GroupByScan> gs;
  hyrise::storage::atable_ptr_t t;
  SumAggregateFun *sum;

 public:

  void BenchmarkSetUp() {
    sm = StorageManager::getInstance();

    t = sm->getTable("order_line");

    gs = std::make_shared<hyrise::access::GroupByScan>();
    gs->setEvent("NO_PAPI");
    gs->addInput(t);

    sum = new SumAggregateFun(t->numberOfColumn("OL_AMOUNT"));

    gs->addFunction(sum);
  }

  void BenchmarkTearDown() {
  }

  GroupByScanBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};

BENCHMARK_F(GroupByScanBase, group_by_tpc_c_delivery) {
  const auto& result = gs->execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_tpc_c_delivery_mat) {
  auto result = gs->execute()->getResultTable();

  hyrise::access::MaterializingScan *ms = new hyrise::access::MaterializingScan(false);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  const auto& result_mat = ms->execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_tpc_c_delivery_mat_memcpy) {
  auto result = gs->execute()->getResultTable();

  hyrise::access::MaterializingScan *ms = new hyrise::access::MaterializingScan(true);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  const auto& result_mat = ms->execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_scan_multiple_fields) {
  hyrise::access::GroupByScan gs2;
  gs2.setEvent("NO_PAPI");
  gs2.addField(0);
  gs2.addField(1);

  gs2.addInput(t);

  hyrise::access::HashBuild hs;
  hs.setEvent("NO_PAPI");
  hs.setKey("groupby");
  hs.addInput(t);

  hs.addField(0);
  hs.addField(1);

  auto group_map = hs.execute()->getResultHashTable();

  gs2.addInputHash(group_map);

  const auto& result = gs2.execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_scan_multiple_fieds_mat) {
  hyrise::access::GroupByScan gs2;
  gs2.setEvent("NO_PAPI");
  gs2.addField(0);
  gs2.addField(1);

  gs2.addInput(t);

  hyrise::access::HashBuild hs;
  hs.setEvent("NO_PAPI");
  hs.setKey("groupby");
  hs.addInput(t);

  hs.addField(0);
  hs.addField(1);

  auto group_map = hs.execute()->getResultHashTable();

  gs2.addInputHash(group_map);

  auto result = gs2.execute()->getResultTable();

  hyrise::access::MaterializingScan ms(false);
  ms.setEvent("NO_PAPI");
  ms.addInput(result);

  const auto& result_mat = ms.execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_scan_multiple_fields_mat_memcpy) {
  hyrise::access::GroupByScan gs2;
  gs2.setEvent("NO_PAPI");
  gs2.addField(0);
  gs2.addField(1);

  gs2.addInput(t);

  hyrise::access::HashBuild hs;
  hs.setEvent("NO_PAPI");
  hs.setKey("groupby");
  hs.addInput(t);

  hs.addField(0);
  hs.addField(1);

  auto group_map = hs.execute()->getResultHashTable();

  gs2.addInputHash(group_map);

  auto result = gs2.execute()->getResultTable();

  hyrise::access::MaterializingScan ms(true);
  ms.setEvent("NO_PAPI");
  ms.addInput(result);

  const auto& result_mat = ms.execute()->getResultTable();
}

