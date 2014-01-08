// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "access/radixjoin/PrefixSum.h"
#include "access/radixjoin/Histogram.h"
#include "access/radixjoin/RadixCluster.h"
#include "helper.h"
#include "io/shortcuts.h"
#include "access/radixjoin/NestedLoopEquiJoin.h"
#include "testing/TableEqualityTest.h"
#include <storage/TableBuilder.h>
#include "access/RadixJoin.h"
#include "access/storage/TableLoad.h"
#include "access/Barrier.h"
#include "taskscheduler/ThreadPerTaskScheduler.h"

namespace hyrise {
namespace access {

using storage::TableBuilder;

class RadixJoinTest : public AccessTest {};

TEST_F(RadixJoinTest, histogram_parallel_4x) {

  auto table = io::Loader::shortcuts::load("test/tables/hash_table_test.tbl");

  Histogram h1;
  h1.addInput(table);
  h1.addField(0);
  h1.setBits(2);    
  h1.setPart(0);
  h1.setCount(4);

  Histogram h2;
  h2.addInput(table);
  h2.addField(0);
  h2.setBits(2);    
  h2.setPart(1);
  h2.setCount(4);

  Histogram h3;
  h3.addInput(table);
  h3.addField(0);
  h3.setBits(2);    
  h3.setPart(2);
  h3.setCount(4);

  Histogram h4;
  h4.addInput(table);
  h4.addField(0);
  h4.setBits(2);    
  h4.setPart(3);
  h4.setCount(4);

  // First Histogram, now Prefix
  h1.execute();  
  h2.execute();  
  h3.execute();  
  h4.execute();  

  PrefixSum p1;
  p1.addInput(h1.getResultTable());
  p1.addInput(h2.getResultTable());
  p1.addInput(h3.getResultTable());
  p1.addInput(h4.getResultTable());
  p1.setPart(0);
  p1.setCount(4);

  PrefixSum p2;
  p2.addInput(h1.getResultTable());
  p2.addInput(h2.getResultTable());
  p2.addInput(h3.getResultTable());
  p2.addInput(h4.getResultTable());
  p2.setPart(1);
  p2.setCount(4);

  PrefixSum p3;
  p3.addInput(h1.getResultTable());
  p3.addInput(h2.getResultTable());
  p3.addInput(h3.getResultTable());
  p3.addInput(h4.getResultTable());
  p3.setPart(2);
  p3.setCount(4);

  PrefixSum p4;
  p4.addInput(h1.getResultTable());
  p4.addInput(h2.getResultTable());
  p4.addInput(h3.getResultTable());
  p4.addInput(h4.getResultTable());
  p4.setPart(3);
  p4.setCount(4);

  p1.execute();
  p2.execute();
  p3.execute();
  p4.execute();

  // Result Prefix SUm
  // Result Histogram
  EXPECT_EQ(0u, p1.getResultTable()->getValueId(0,0).valueId);
  EXPECT_EQ(3u, p1.getResultTable()->getValueId(0,1).valueId);
  EXPECT_EQ(7u, p1.getResultTable()->getValueId(0,2).valueId);
  EXPECT_EQ(8u, p1.getResultTable()->getValueId(0,3).valueId);

  EXPECT_EQ(1u, p2.getResultTable()->getValueId(0,0).valueId);
  EXPECT_EQ(4u, p2.getResultTable()->getValueId(0,1).valueId);
  EXPECT_EQ(7u, p2.getResultTable()->getValueId(0,2).valueId);
  EXPECT_EQ(8u, p2.getResultTable()->getValueId(0,3).valueId);

  EXPECT_EQ(3u, p3.getResultTable()->getValueId(0,0).valueId);
  EXPECT_EQ(4u, p3.getResultTable()->getValueId(0,1).valueId);
  EXPECT_EQ(7u, p3.getResultTable()->getValueId(0,2).valueId);
  EXPECT_EQ(8u, p3.getResultTable()->getValueId(0,3).valueId);

  EXPECT_EQ(3u, p4.getResultTable()->getValueId(0,0).valueId);
  EXPECT_EQ(5u, p4.getResultTable()->getValueId(0,1).valueId);
  EXPECT_EQ(8u, p4.getResultTable()->getValueId(0,2).valueId);
  EXPECT_EQ(8u, p4.getResultTable()->getValueId(0,3).valueId);

  // Result Histogram
  EXPECT_EQ(1u, h1.getResultTable()->getValueId(0,0).valueId);
  EXPECT_EQ(1u, h1.getResultTable()->getValueId(0,1).valueId);
  EXPECT_EQ(0u, h1.getResultTable()->getValueId(0,2).valueId);
  EXPECT_EQ(0u, h1.getResultTable()->getValueId(0,3).valueId);

  EXPECT_EQ(2u, h2.getResultTable()->getValueId(0,0).valueId);
  EXPECT_EQ(0u, h2.getResultTable()->getValueId(0,1).valueId);
  EXPECT_EQ(0u, h2.getResultTable()->getValueId(0,2).valueId);
  EXPECT_EQ(0u, h2.getResultTable()->getValueId(0,3).valueId);

  EXPECT_EQ(0u, h3.getResultTable()->getValueId(0,0).valueId);
  EXPECT_EQ(1u, h3.getResultTable()->getValueId(0,1).valueId);
  EXPECT_EQ(1u, h3.getResultTable()->getValueId(0,2).valueId);
  EXPECT_EQ(0u, h3.getResultTable()->getValueId(0,3).valueId);

  EXPECT_EQ(0u, h4.getResultTable()->getValueId(0,0).valueId);
  EXPECT_EQ(2u, h4.getResultTable()->getValueId(0,1).valueId);
  EXPECT_EQ(0u, h4.getResultTable()->getValueId(0,2).valueId);
  EXPECT_EQ(0u, h4.getResultTable()->getValueId(0,3).valueId);

}


TEST_F(RadixJoinTest, check_prefixsum) {
  // create input Table
  auto t1 = io::Loader::shortcuts::load("test/prefix_sum.tbl");
  auto reference = io::Loader::shortcuts::load("test/prefix_sum_result.tbl");

  PrefixSum ps;
  ps.addInput(t1);
  ps.execute();

  auto result = ps.getResultTable();

  for(size_t i = 0; i < t1->size(); i++)
    ASSERT_TRUE(result->getValueId(0, i).valueId == reference->getValue<unsigned int>(0, i)) << result->getValueId(0, i).valueId << " != " << reference->getValue<unsigned int>(0, i) << " at " << i;
}

TEST_F(RadixJoinTest, check_prefixsum_parallel_p3) {
  TableBuilder::param_list list;
  list.append().set_type("INTEGER").set_name("count");
  auto t1 = TableBuilder::build(list, false);
  auto t2 = TableBuilder::build(list, false);
  auto t3 = TableBuilder::build(list, false);

  t1->resize(2); t2->resize(2); t3->resize(2);
  t1->setValueId(0,0, {10,0});
  t1->setValueId(0,1, {20,0});

  t2->setValueId(0,0, {3,0});
  t2->setValueId(0,1, {7,0});

  t3->setValueId(0,0, {8,0});
  t3->setValueId(0,1, {3,0});

  PrefixSum ps;
  ps.addInput(t1);
  ps.addInput(t2);
  ps.addInput(t3);
  ps.setPart(2);
  ps.execute();

  auto result = ps.getResultTable();

  EXPECT_EQ(13u, result->getValueId(0,0).valueId);
  EXPECT_EQ(48u, result->getValueId(0,1).valueId);
  
}

TEST_F(RadixJoinTest, check_prefixsum_parallel_p2) {
  TableBuilder::param_list list;
  list.append().set_type("INTEGER").set_name("count");
  auto t1 = TableBuilder::build(list, false);
  auto t2 = TableBuilder::build(list, false);
  auto t3 = TableBuilder::build(list, false);

  t1->resize(2); t2->resize(2); t3->resize(2);
  t1->setValueId(0,0, {10,0});
  t1->setValueId(0,1, {20,0});

  t2->setValueId(0,0, {3,0});
  t2->setValueId(0,1, {7,0});

  t3->setValueId(0,0, {8,0});
  t3->setValueId(0,1, {3,0});

  PrefixSum ps;
  ps.addInput(t1);
  ps.addInput(t2);
  ps.addInput(t3);
  ps.setPart(1);
  ps.execute();

  auto result = ps.getResultTable();

  EXPECT_EQ(10u, result->getValueId(0,0).valueId);
  EXPECT_EQ(41u, result->getValueId(0,1).valueId);  
}

TEST_F(RadixJoinTest, check_prefixsum_parallel_p1) {
  TableBuilder::param_list list;
  list.append().set_type("INTEGER").set_name("count");
  auto t1 = TableBuilder::build(list, false);
  auto t2 = TableBuilder::build(list, false);
  auto t3 = TableBuilder::build(list, false);

  t1->resize(2); t2->resize(2); t3->resize(2);
  t1->setValueId(0,0, {10,0});
  t1->setValueId(0,1, {20,0});

  t2->setValueId(0,0, {3,0});
  t2->setValueId(0,1, {7,0});

  t3->setValueId(0,0, {8,0});
  t3->setValueId(0,1, {3,0});

  PrefixSum ps;
  ps.addInput(t1);
  ps.addInput(t2);
  ps.addInput(t3);
  ps.setPart(0);
  ps.execute();

  auto result = ps.getResultTable();

  EXPECT_EQ(0u, result->getValueId(0,0).valueId);
  EXPECT_EQ(21u, result->getValueId(0,1).valueId);
  
}

TEST_F(RadixJoinTest, check_prefixsum_parallel_merge) {
  TableBuilder::param_list list;
  list.append().set_type("INTEGER").set_name("count");
  auto t1 = TableBuilder::build(list, false);
  auto t2 = TableBuilder::build(list, false);
  auto t3 = TableBuilder::build(list, false);

  t1->resize(2); t2->resize(2); t3->resize(2);
  t1->setValueId(0,0, {0,0});
  t1->setValueId(0,1, {10,0});

  t2->setValueId(0,0, {3,0});
  t2->setValueId(0,1, {11,0});

  t3->setValueId(0,0, {8,0});
  t3->setValueId(0,1, {18,0});

  MergePrefixSum ps;
  ps.addInput(t1);
  ps.addInput(t2);
  ps.addInput(t3);
  ps.execute();

  auto result = ps.getResultTable();

  EXPECT_EQ(0u, result->getValueId(0,0).valueId);
  EXPECT_EQ(10u, result->getValueId(0,1).valueId);
  
}

TEST_F(RadixJoinTest, hist_prefix_radix_cluster) {

  auto table = io::Loader::shortcuts::load("test/tables/hash_table_test.tbl");
  hyrise::access::Histogram hst;
  hst.setBits(2);
  hst.addField(0);
  hst.addInput(table);
  hst.execute();

  auto result = hst.getResultTable();
  EXPECT_EQ(3u, result->getValueId(0,0).valueId);
  EXPECT_EQ(4u, result->getValueId(0,1).valueId);
  EXPECT_EQ(1u, result->getValueId(0,2).valueId);
  EXPECT_EQ(0u, result->getValueId(0,3).valueId);

  PrefixSum ps;
  ps.addInput(hst.getResultTable());
  ps.execute();

  result = ps.getResultTable();
  EXPECT_EQ(0u, result->getValueId(0,0).valueId);
  EXPECT_EQ(3u, result->getValueId(0,1).valueId);
  EXPECT_EQ(7u, result->getValueId(0,2).valueId);
  EXPECT_EQ(8u, result->getValueId(0,3).valueId);

  CreateRadixTable c;
  c.addInput(table);
  c.execute();
  auto restab = c.getResultTable();

  RadixCluster rx;
  rx.setBits(2);
  rx.addField(0);
  rx.addInput(table);
  rx.addInput(restab);
  rx.addInput(ps.getResultTable());
  rx.execute();

  result = rx.getResultTable();
  
  EXPECT_EQ(table->size(), result->size());
  EXPECT_EQ(0u, result->getValueId(0,0).valueId);
  EXPECT_EQ(0u, result->getValueId(0,1).valueId);
  EXPECT_EQ(0u, result->getValueId(0,2).valueId);
  EXPECT_EQ(1u, result->getValueId(0,3).valueId);
  EXPECT_EQ(1u, result->getValueId(0,4).valueId);
  EXPECT_EQ(1u, result->getValueId(0,5).valueId);
  EXPECT_EQ(1u, result->getValueId(0,6).valueId);
  EXPECT_EQ(2u, result->getValueId(0,7).valueId);
  
}

TEST_F(RadixJoinTest, hist_prefix_radix_cluster_string) {

  auto table = io::Loader::shortcuts::load("test/tables/hash_table_test.tbl");
  hyrise::access::Histogram hst;
  hst.setBits(2);
  hst.addField(1);
  hst.addInput(table);
  hst.execute();

  auto result = hst.getResultTable();
  EXPECT_EQ(3u, result->getValueId(0,0).valueId);
  EXPECT_EQ(0u, result->getValueId(0,1).valueId);
  EXPECT_EQ(5u, result->getValueId(0,2).valueId);
  EXPECT_EQ(0u, result->getValueId(0,3).valueId);

  PrefixSum ps;
  ps.addInput(hst.getResultTable());
  ps.execute();

  result = ps.getResultTable();
  EXPECT_EQ(0u, result->getValueId(0,0).valueId);
  EXPECT_EQ(3u, result->getValueId(0,1).valueId);
  EXPECT_EQ(3u, result->getValueId(0,2).valueId);
  EXPECT_EQ(8u, result->getValueId(0,3).valueId);

  CreateRadixTable c;
  c.addInput(table);
  c.execute();
  auto restab = c.getResultTable();

  RadixCluster rx;
  rx.setBits(2);
  rx.addField(1);
  rx.addInput(table);
  rx.addInput(restab);
  rx.addInput(ps.getResultTable());
  rx.execute();

  result = rx.getResultTable();
  
  EXPECT_EQ(table->size(), result->size());
  EXPECT_EQ((value_id_t)std::hash<std::string>()("B"), result->getValueId(0,0).valueId);
  EXPECT_EQ((value_id_t)std::hash<std::string>()("B"), result->getValueId(0,1).valueId);
  EXPECT_EQ((value_id_t)std::hash<std::string>()("B"), result->getValueId(0,2).valueId);
  EXPECT_EQ((value_id_t)std::hash<std::string>()("A"), result->getValueId(0,3).valueId);
  EXPECT_EQ((value_id_t)std::hash<std::string>()("A"), result->getValueId(0,4).valueId);
  EXPECT_EQ((value_id_t)std::hash<std::string>()("A"), result->getValueId(0,5).valueId);
  EXPECT_EQ((value_id_t)std::hash<std::string>()("C"), result->getValueId(0,6).valueId);
  EXPECT_EQ((value_id_t)std::hash<std::string>()("A"), result->getValueId(0,7).valueId);
  
}

TEST_F(RadixJoinTest, multi_pass_radix_cluster) {
  auto table = io::Loader::shortcuts::load("test/tables/radix_cluster_mpass.tbl");

  // Create a histogram
  Histogram hst;
  hst.setBits(2);
  hst.addField(0);
  hst.addInput(table);
  hst.execute();
  auto histo_result = hst.getResultTable();

  EXPECT_EQ(4u, histo_result->size());
  EXPECT_EQ(5u, histo_result->getValueId(0,0).valueId);
  EXPECT_EQ(5u, histo_result->getValueId(0,1).valueId);
  EXPECT_EQ(1u, histo_result->getValueId(0,2).valueId);
  EXPECT_EQ(1u, histo_result->getValueId(0,3).valueId);
  
  // Creates the prefix sums with the offsets for the clustering
  PrefixSum ps;
  ps.addInput(histo_result);
  ps.execute();
  auto prefix_result = ps.getResultTable();

  EXPECT_EQ(4u, prefix_result->size());
  EXPECT_EQ(0u, prefix_result->getValueId(0,0).valueId);
  EXPECT_EQ(5u, prefix_result->getValueId(0,1).valueId);
  EXPECT_EQ(10u, prefix_result->getValueId(0,2).valueId);
  EXPECT_EQ(11u, prefix_result->getValueId(0,3).valueId);

  CreateRadixTable c;
  c.addInput(table);
  c.execute();

  CreateRadixTable c2;
  c2.addInput(table);
  c2.execute();

  // Performs the clustering
  RadixCluster rx;
  rx.setBits(2);
  rx.addField(0);
  rx.addInput(table);
  rx.addInput(c.getResultTable());
  rx.addInput(prefix_result);
  rx.execute();
  auto pass1 = rx.getResultTable();

  EXPECT_EQ(0u, pass1->getValueId(0,0).valueId);
  EXPECT_EQ(4u, pass1->getValueId(0,1).valueId);
  EXPECT_EQ(0u, pass1->getValueId(0,2).valueId);
  EXPECT_EQ(0u, pass1->getValueId(0,3).valueId);
  EXPECT_EQ(4u, pass1->getValueId(0,4).valueId);
  EXPECT_EQ(1u, pass1->getValueId(0,5).valueId);
  EXPECT_EQ(1u, pass1->getValueId(0,6).valueId);
  EXPECT_EQ(1u, pass1->getValueId(0,7).valueId);
  EXPECT_EQ(25u, pass1->getValueId(0,8).valueId);
  EXPECT_EQ(9u, pass1->getValueId(0,9).valueId);
  EXPECT_EQ(2u, pass1->getValueId(0,10).valueId);
  EXPECT_EQ(3u, pass1->getValueId(0,11).valueId);

  // We know the number of buckets from the previous operation
  Histogram2ndPass hst2;
  hst2.setBits(2,0);
  hst2.setBits2(1,2);
  hst2.addField(0);
  hst2.addInput(pass1);
  hst2.execute();
  auto pass2 = hst2.getResultTable();

  EXPECT_EQ(prefix_result->size() * 2, pass2->size());
  EXPECT_EQ(3u, pass2->getValueId(0,0).valueId);
  EXPECT_EQ(2u, pass2->getValueId(0,1).valueId);

  EXPECT_EQ(5u, pass2->getValueId(0,2).valueId);
  EXPECT_EQ(0u, pass2->getValueId(0,3).valueId);

  EXPECT_EQ(1u, pass2->getValueId(0,4).valueId);
  EXPECT_EQ(0u, pass2->getValueId(0,5).valueId);

  EXPECT_EQ(1u, pass2->getValueId(0,6).valueId);
  EXPECT_EQ(0u, pass2->getValueId(0,7).valueId);

  PrefixSum ps2;
  ps2.addInput(pass2);
  ps2.execute();
  auto prefix2 = ps2.getResultTable();
  EXPECT_EQ(prefix_result->size() * 2, prefix2->size());
  EXPECT_EQ(0u, prefix2->getValueId(0,0).valueId);
  EXPECT_EQ(3u, prefix2->getValueId(0,1).valueId);

  EXPECT_EQ(5u, prefix2->getValueId(0,2).valueId);
  EXPECT_EQ(10u, prefix2->getValueId(0,3).valueId);

  EXPECT_EQ(10u, prefix2->getValueId(0,4).valueId);
  EXPECT_EQ(11u, prefix2->getValueId(0,5).valueId);

  EXPECT_EQ(11u, prefix2->getValueId(0,6).valueId);
  EXPECT_EQ(12u, prefix2->getValueId(0,7).valueId);

  RadixCluster2ndPass rx2;
  rx2.setBits1(2, 0);
  rx2.setBits2(1, 2);
  rx2.addInput(pass1);
  rx2.addInput(c2.getResultTable());
  rx2.addInput(prefix2);
  rx2.execute();
  auto pass_rx2 = rx2.getResultTable();

  EXPECT_EQ(0u, pass_rx2->getValueId(0,0).valueId);
  EXPECT_EQ(0u, pass_rx2->getValueId(0,1).valueId);
  EXPECT_EQ(0u, pass_rx2->getValueId(0,2).valueId);
  EXPECT_EQ(4u, pass_rx2->getValueId(0,3).valueId);
  EXPECT_EQ(4u, pass_rx2->getValueId(0,4).valueId);
  EXPECT_EQ(1u, pass_rx2->getValueId(0,5).valueId);
  EXPECT_EQ(1u, pass_rx2->getValueId(0,6).valueId);
  EXPECT_EQ(1u, pass_rx2->getValueId(0,7).valueId);
  EXPECT_EQ(25u,pass_rx2->getValueId(0,8).valueId);
  EXPECT_EQ(9u, pass_rx2->getValueId(0,9).valueId);
  EXPECT_EQ(2u, pass_rx2->getValueId(0,10).valueId);
  EXPECT_EQ(3u, pass_rx2->getValueId(0,11).valueId);
}

TEST_F(RadixJoinTest, multi_pass_radix_cluster_parallel) {
  auto table = io::Loader::shortcuts::load("test/tables/radix_cluster_mpass.tbl");

  // Create a histogram
  Histogram hst;
  hst.setBits(2);
  hst.addField(0);
  hst.addInput(table);
  hst.setPart(0);
  hst.setCount(2);
  hst.execute();
  auto histo_result = hst.getResultTable();

  EXPECT_EQ(4u, histo_result->size());
  EXPECT_EQ(4u, histo_result->getValueId(0,0).valueId);
  EXPECT_EQ(2u, histo_result->getValueId(0,1).valueId);
  EXPECT_EQ(0u, histo_result->getValueId(0,2).valueId);
  EXPECT_EQ(0u, histo_result->getValueId(0,3).valueId);

  Histogram hst2;
  hst2.setBits(2);
  hst2.addField(0);
  hst2.addInput(table);
  hst2.setPart(1);
  hst2.setCount(2);
  hst2.execute();
  auto histo_result2 = hst2.getResultTable();

  EXPECT_EQ(4u, histo_result2->size());
  EXPECT_EQ(1u, histo_result2->getValueId(0,0).valueId);
  EXPECT_EQ(3u, histo_result2->getValueId(0,1).valueId);
  EXPECT_EQ(1u, histo_result2->getValueId(0,2).valueId);
  EXPECT_EQ(1u, histo_result2->getValueId(0,3).valueId);

  // Creates the prefix sums with the offsets for the clustering
  PrefixSum ps;
  ps.addInput(histo_result);
  ps.addInput(histo_result2);
  ps.setPart(0);
  ps.setCount(2);
  ps.execute();
  auto prefix_result = ps.getResultTable();

  EXPECT_EQ(4u, prefix_result->size());
  EXPECT_EQ(0u, prefix_result->getValueId(0,0).valueId);
  EXPECT_EQ(5u, prefix_result->getValueId(0,1).valueId);
  EXPECT_EQ(10u, prefix_result->getValueId(0,2).valueId);
  EXPECT_EQ(11u, prefix_result->getValueId(0,3).valueId);

  PrefixSum ps2;
  ps2.addInput(histo_result);
  ps2.addInput(histo_result2);
  ps2.setPart(1);
  ps2.setCount(2);
  ps2.execute();
  auto prefix_result2 = ps2.getResultTable();

  EXPECT_EQ(4u, prefix_result2->size());
  EXPECT_EQ(4u, prefix_result2->getValueId(0,0).valueId);
  EXPECT_EQ(7u, prefix_result2->getValueId(0,1).valueId);
  EXPECT_EQ(10u, prefix_result2->getValueId(0,2).valueId);
  EXPECT_EQ(11u, prefix_result2->getValueId(0,3).valueId);

  CreateRadixTable c;
  c.addInput(table);
  c.execute();

  CreateRadixTable c2;
  c2.addInput(table);
  c2.execute();

  // // Performs the clustering
  RadixCluster rx;
  rx.setBits(2);
  rx.addField(0);
  rx.addInput(table);
  rx.addInput(c.getResultTable());
  rx.addInput(prefix_result);
  rx.setPart(0);
  rx.setCount(2);
  rx.execute();
  auto pass1_1 = rx.getResultTable();

  RadixCluster rx2;
  rx2.setBits(2);
  rx2.addField(0);
  rx2.addInput(table);
  rx2.addInput(c.getResultTable());
  rx2.addInput(prefix_result2);
  rx2.setPart(1);
  rx2.setCount(2);
  rx2.execute();
  auto pass1_2 = rx2.getResultTable();

  EXPECT_EQ(pass1_1.get(), pass1_2.get());

  EXPECT_EQ(0u, pass1_1->getValueId(0,0).valueId);
  EXPECT_EQ(4u, pass1_1->getValueId(0,1).valueId);
  EXPECT_EQ(0u, pass1_1->getValueId(0,2).valueId);
  EXPECT_EQ(0u, pass1_1->getValueId(0,3).valueId);
  EXPECT_EQ(4u, pass1_1->getValueId(0,4).valueId);
  EXPECT_EQ(1u, pass1_1->getValueId(0,5).valueId);

  EXPECT_EQ(1u, pass1_1->getValueId(0,6).valueId);
  EXPECT_EQ(1u, pass1_1->getValueId(0,7).valueId);
  EXPECT_EQ(25u, pass1_1->getValueId(0,8).valueId);
  EXPECT_EQ(9u, pass1_1->getValueId(0,9).valueId);
  EXPECT_EQ(2u, pass1_1->getValueId(0,10).valueId);
  EXPECT_EQ(3u, pass1_1->getValueId(0,11).valueId);

  // // We know the number of buckets from the previous operation
  Histogram2ndPass hst2_1;
  hst2_1.setBits(2);
  hst2_1.setBits2(1,2);
  hst2_1.addField(0);
  hst2_1.addInput(pass1_1);
  hst2_1.setPart(0);
  hst2_1.setCount(2);
  hst2_1.execute();
  auto pass2_1 = hst2_1.getResultTable();

  EXPECT_EQ(prefix_result->size() * 2, pass2_1->size());
  EXPECT_EQ(3u, pass2_1->getValueId(0,0).valueId);
  EXPECT_EQ(2u, pass2_1->getValueId(0,1).valueId);

  EXPECT_EQ(1u, pass2_1->getValueId(0,2).valueId);
  EXPECT_EQ(0u, pass2_1->getValueId(0,3).valueId);

  EXPECT_EQ(0u, pass2_1->getValueId(0,4).valueId);
  EXPECT_EQ(0u, pass2_1->getValueId(0,5).valueId);

  EXPECT_EQ(0u, pass2_1->getValueId(0,6).valueId);
  EXPECT_EQ(0u, pass2_1->getValueId(0,7).valueId);


  Histogram2ndPass hst2_2;
  hst2_2.setBits(2);
  hst2_2.setBits2(1,2);
  hst2_2.addField(0);
  hst2_2.addInput(pass1_1);
  hst2_2.setPart(1);
  hst2_2.setCount(2);
  hst2_2.execute();
  auto pass2_2 = hst2_2.getResultTable();

  EXPECT_EQ(prefix_result->size() * 2, pass2_2->size());
  EXPECT_EQ(0u, pass2_2->getValueId(0,0).valueId);
  EXPECT_EQ(0u, pass2_2->getValueId(0,1).valueId);

  EXPECT_EQ(4u, pass2_2->getValueId(0,2).valueId);
  EXPECT_EQ(0u, pass2_2->getValueId(0,3).valueId);

  EXPECT_EQ(1u, pass2_2->getValueId(0,4).valueId);
  EXPECT_EQ(0u, pass2_2->getValueId(0,5).valueId);

  EXPECT_EQ(1u, pass2_2->getValueId(0,6).valueId);
  EXPECT_EQ(0u, pass2_2->getValueId(0,7).valueId);

  PrefixSum ps2_1;
  ps2_1.addInput(pass2_1);
  ps2_1.addInput(pass2_2);
  ps2_1.setPart(0);
  ps2_1.setCount(2);
  ps2_1.execute();
  auto prefix2_1 = ps2_1.getResultTable();
  EXPECT_EQ(prefix_result->size() * 2, prefix2_1->size());
  EXPECT_EQ(0u, prefix2_1->getValueId(0,0).valueId);
  EXPECT_EQ(3u, prefix2_1->getValueId(0,1).valueId);

  EXPECT_EQ(5u, prefix2_1->getValueId(0,2).valueId);
  EXPECT_EQ(10u, prefix2_1->getValueId(0,3).valueId);

  EXPECT_EQ(10u, prefix2_1->getValueId(0,4).valueId);
  EXPECT_EQ(11u, prefix2_1->getValueId(0,5).valueId);

  EXPECT_EQ(11u, prefix2_1->getValueId(0,6).valueId);
  EXPECT_EQ(12u, prefix2_1->getValueId(0,7).valueId);

  PrefixSum ps2_2;
  ps2_2.addInput(pass2_1);
  ps2_2.addInput(pass2_2);
  ps2_2.setPart(1);
  ps2_2.setCount(2);
  ps2_2.execute();
  auto prefix2_2 = ps2_2.getResultTable();
  EXPECT_EQ(prefix_result->size() * 2, prefix2_2->size());
  EXPECT_EQ(3u, prefix2_2->getValueId(0,0).valueId);
  EXPECT_EQ(5u, prefix2_2->getValueId(0,1).valueId);

  EXPECT_EQ(6u, prefix2_2->getValueId(0,2).valueId);
  EXPECT_EQ(10u, prefix2_2->getValueId(0,3).valueId);

  EXPECT_EQ(10u, prefix2_2->getValueId(0,4).valueId);
  EXPECT_EQ(11u, prefix2_2->getValueId(0,5).valueId);

  EXPECT_EQ(11u, prefix2_2->getValueId(0,6).valueId);
  EXPECT_EQ(12u, prefix2_2->getValueId(0,7).valueId);

  RadixCluster2ndPass rx2_1;
  rx2_1.setBits1(2, 0);
  rx2_1.setBits2(1, 2);
  rx2_1.addInput(pass1_1);
  rx2_1.addInput(c2.getResultTable());
  rx2_1.addInput(prefix2_1);
  rx2_1.setPart(0);
  rx2_1.setCount(2);
  rx2_1.execute();
  auto pass_rx2_1 = rx2_1.getResultTable();

  RadixCluster2ndPass rx2_2;
  rx2_2.setBits1(2, 0);
  rx2_2.setBits2(1, 2);
  rx2_2.addInput(pass1_2);
  rx2_2.addInput(c2.getResultTable());
  rx2_2.addInput(prefix2_2);
  rx2_2.setPart(1);
  rx2_2.setCount(2);
  rx2_2.execute();
  auto pass_rx2_2 = rx2_2.getResultTable();

  EXPECT_EQ(0u, pass_rx2_1->getValueId(0,0).valueId);
  EXPECT_EQ(0u, pass_rx2_1->getValueId(0,1).valueId);
  EXPECT_EQ(0u, pass_rx2_1->getValueId(0,2).valueId);
  EXPECT_EQ(4u, pass_rx2_1->getValueId(0,3).valueId);
  EXPECT_EQ(4u, pass_rx2_1->getValueId(0,4).valueId);
  EXPECT_EQ(1u, pass_rx2_1->getValueId(0,5).valueId);
  EXPECT_EQ(1u, pass_rx2_1->getValueId(0,6).valueId);
  EXPECT_EQ(1u, pass_rx2_1->getValueId(0,7).valueId);
  EXPECT_EQ(25u,pass_rx2_1->getValueId(0,8).valueId);
  EXPECT_EQ(9u, pass_rx2_1->getValueId(0,9).valueId);
  EXPECT_EQ(2u, pass_rx2_1->getValueId(0,10).valueId);
  EXPECT_EQ(3u, pass_rx2_1->getValueId(0,11).valueId);

  MergePrefixSum mprx;
  mprx.addInput(prefix2_1);
  mprx.addInput(prefix2_2);
  mprx.execute();
  auto merged_prx = mprx.getResultTable();

  EXPECT_EQ(prefix2_1->size(), merged_prx->size());
  EXPECT_EQ(0u, merged_prx->getValueId(0,0).valueId);
  EXPECT_EQ(3u, merged_prx->getValueId(0,1).valueId);

  EXPECT_EQ(5u, merged_prx->getValueId(0,2).valueId);
  EXPECT_EQ(10u, merged_prx->getValueId(0,3).valueId);

  EXPECT_EQ(10u, merged_prx->getValueId(0,4).valueId);
  EXPECT_EQ(11u, merged_prx->getValueId(0,5).valueId);

  EXPECT_EQ(11u, merged_prx->getValueId(0,6).valueId);
  EXPECT_EQ(12u, merged_prx->getValueId(0,7).valueId);
}

TEST_F(RadixJoinTest, determine_dynamic_count) {
  size_t MTS = 100;
  auto tbl = io::Loader::shortcuts::load("test/tables/companies.tbl");

  auto resizedTbl = tbl->copy_structure();
  resizedTbl->resize(100000); /// 100k
  auto fakeTask1 = std::shared_ptr<PlanOperation>(new Barrier());
  auto fakeTask2 = std::shared_ptr<PlanOperation>(new Barrier());
  fakeTask1->addInput(resizedTbl);
  fakeTask1->addField(0);
  fakeTask2->addInput(resizedTbl);
  fakeTask2->addField(0);
  (*fakeTask1)();
  (*fakeTask2)();
  std::shared_ptr<PlanOperation> radix = std::shared_ptr<PlanOperation>(new RadixJoin());
  radix->addField(0);
  radix->addField(0);
  radix->addDependency(fakeTask1);
  radix->addDependency(fakeTask2); 
  auto dynamicCount1 = radix->determineDynamicCount(MTS);

  auto resizedTbl2 = tbl->copy_structure();
  resizedTbl2->resize(10000000); // 10 million
  auto fakeTask3 = std::shared_ptr<PlanOperation>(new Barrier());
  auto fakeTask4 = std::shared_ptr<PlanOperation>(new Barrier());
  fakeTask3->addInput(resizedTbl2);
  fakeTask3->addField(0);
  fakeTask4->addInput(resizedTbl2);
  fakeTask4->addField(0);
  (*fakeTask3)();
  (*fakeTask4)();
  std::shared_ptr<PlanOperation> radix2 = std::shared_ptr<PlanOperation>(new RadixJoin());
  radix2->addField(0);
  radix2->addField(0);
  radix2->addDependency(fakeTask3);
  radix2->addDependency(fakeTask4); 
  auto dynamicCount2 = radix2->determineDynamicCount(MTS);

  ASSERT_GT(dynamicCount2, dynamicCount1);
}

class RadixDynamicCountTest : public AccessTest, public ::testing::WithParamInterface<int> {
  protected:
    virtual void SetUp() {
      auto companies = std::make_shared<TableLoad>();
      companies->setFileName("tables/companies.tbl");
      companies->setTableName("companies");
      (*companies)();
      
      auto employees = std::make_shared<TableLoad>();
      employees->setFileName("tables/employees.tbl");
      employees->setTableName("employees");
      (*employees)();

      radix = std::make_shared<RadixJoin>();
      radix->addField(0);
      radix->addField(1);
      radix->setBits1(4);
      radix->setBits2(4);
      radix->setOperatorId("testRadix");
      radix->addDependency(companies);
      radix->addDependency(employees);

      // Adding the waiter before applyDynamicParallelization ensures,
      // that the task with radix as dependency get properly rerouted.
      waiter = std::make_shared<taskscheduler::WaitTask>();
      waiter->addDependency(radix);

      auto dynamicCount = GetParam();

      tasks = radix->applyDynamicParallelization(dynamicCount);

    }

    std::shared_ptr<RadixJoin> radix;
    std::shared_ptr<taskscheduler::WaitTask> waiter;
    std::vector<taskscheduler::task_ptr_t> tasks;
};

TEST_P(RadixDynamicCountTest, virtual_radix_no_longer_dependency) {
  // Radix should no longer be a dependency of the waiter.
  // It should have been replaced by a concrete taks of the radix transformation.
  ASSERT_FALSE(waiter->isDependency(radix));
}

TEST_P(RadixDynamicCountTest, concrete_radix_is_dependency) {
  auto finalOp = std::dynamic_pointer_cast<PlanOperation>(tasks.back());

  // The final op of the radix transformation tasks should be a new dependency of the waiter
  ASSERT_TRUE(waiter->isDependency(finalOp));
}

TEST_P(RadixDynamicCountTest, execute_and_check_result) {
  auto finalOp = std::dynamic_pointer_cast<PlanOperation>(tasks.back());
  tasks.push_back(waiter);

  auto scheduler = std::make_shared<taskscheduler::ThreadPerTaskScheduler>();
  scheduler->scheduleQuery(tasks);

  waiter->wait();

  auto resultTable = finalOp->getResultTable();
  auto referenceTable = io::Loader::shortcuts::load("test/tables/companies_employees_joined.tbl");

  ASSERT_TRUE(referenceTable->contentEquals(resultTable));
}

INSTANTIATE_TEST_CASE_P(RadixJoinDynamicParallelizationTest, RadixDynamicCountTest, ::testing::Values(1, 4));

}}
