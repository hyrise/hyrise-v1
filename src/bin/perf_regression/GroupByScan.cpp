// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <access.h>
#include <storage.h>
#include <io.h>

namespace hyrise {
namespace access {

// GroupByScan as derived from TPC-C Delivery Transaction
// See TPC-C Reference Section A.4

class GroupByScanBase : public ::testing::Benchmark {

 protected:

  io::StorageManager *sm;
  std::shared_ptr<GroupByScan> gs;
  storage::atable_ptr_t t;
  SumAggregateFun *sum;

 public:

  void BenchmarkSetUp() {
    sm = io::StorageManager::getInstance();

    t = sm->getTable("order_line");

    gs = std::make_shared<GroupByScan>();
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
  gs->execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_tpc_c_delivery_mat) {
  auto result = gs->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(false);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  ms->execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_tpc_c_delivery_mat_memcpy) {
  auto result = gs->execute()->getResultTable();

  MaterializingScan *ms = new MaterializingScan(true);
  ms->setEvent("NO_PAPI");
  ms->addInput(result);

  ms->execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_scan_multiple_fields) {
  GroupByScan gs2;
  gs2.setEvent("NO_PAPI");
  gs2.addField(0);
  gs2.addField(1);

  gs2.addInput(t);

  HashBuild hs;
  hs.setEvent("NO_PAPI");
  hs.setKey("groupby");
  hs.addInput(t);

  hs.addField(0);
  hs.addField(1);

  auto group_map = hs.execute()->getResultHashTable();

  gs2.addInput(group_map);

  gs2.execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_scan_multiple_fieds_mat) {
  GroupByScan gs2;
  gs2.setEvent("NO_PAPI");
  gs2.addField(0);
  gs2.addField(1);

  gs2.addInput(t);

  HashBuild hs;
  hs.setEvent("NO_PAPI");
  hs.setKey("groupby");
  hs.addInput(t);

  hs.addField(0);
  hs.addField(1);

  auto group_map = hs.execute()->getResultHashTable();

  gs2.addInput(group_map);

  auto result = gs2.execute()->getResultTable();

  MaterializingScan ms(false);
  ms.setEvent("NO_PAPI");
  ms.addInput(result);

  ms.execute()->getResultTable();
}

BENCHMARK_F(GroupByScanBase, group_by_scan_multiple_fields_mat_memcpy) {
  GroupByScan gs2;
  gs2.setEvent("NO_PAPI");
  gs2.addField(0);
  gs2.addField(1);

  gs2.addInput(t);

  HashBuild hs;
  hs.setEvent("NO_PAPI");
  hs.setKey("groupby");
  hs.addInput(t);

  hs.addField(0);
  hs.addField(1);

  auto group_map = hs.execute()->getResultHashTable();

  gs2.addInput(group_map);

  auto result = gs2.execute()->getResultTable();

  MaterializingScan ms(true);
  ms.setEvent("NO_PAPI");
  ms.addInput(result);

  ms.execute()->getResultTable();

}

}
}
