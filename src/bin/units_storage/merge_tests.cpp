// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "testing/TableEqualityTest.h"


#include <cstdlib>

#include <io.h>
#include <io/shortcuts.h>

#include <storage.h>

#include <helper/PapiTracer.h>
#include <helper/types.h>
#include <helper/vector_helpers.h>

namespace hyrise { namespace storage {

class MergeTests : public ::hyrise::Test {};


TEST_F(MergeTests, simple_merge_test_with_rows) {
  auto main = io::Loader::shortcuts::load("test/merge1_main_row.tbl");
  auto delta = io::Loader::shortcuts::load("test/merge1_delta.tbl"); 
  auto correct_result = io::Loader::shortcuts::load("test/merge1_result.tbl");

  std::vector<storage::c_atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger(new DefaultMergeStrategy(), new SequentialHeapMerger());

  const auto& result = merger.merge(tables);

  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);
}



TEST_F(MergeTests, simple_merge_test) {
  auto main = io::Loader::shortcuts::load("test/merge1_main.tbl");
  auto delta = io::Loader::shortcuts::load("test/merge1_delta.tbl");
  auto correct_result = io::Loader::shortcuts::load("test/merge1_result.tbl");

  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger(new DefaultMergeStrategy(), new SequentialHeapMerger());

  const auto& result = merger.merge(tables);

  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

}

TEST_F(MergeTests, simple_merge_test_valid_rows) {
  auto main = io::Loader::shortcuts::load("test/merge1_main.tbl");
  auto delta = io::Loader::shortcuts::load("test/merge1_delta.tbl");
  auto correct_result = io::Loader::shortcuts::load("test/merge1_result.tbl");

  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger(new DefaultMergeStrategy(), new SequentialHeapMerger());

  // First test with no valid
  std::vector<bool> validOne(main->size() + delta->size(), false);  
  auto result = merger.merge(tables, true, validOne);
  ASSERT_EQ(0u, result[0]->size());

  // Second test with only some valid
  std::vector<bool> valid(main->size() + delta->size(), false);
  valid[3] = true;
  valid[valid.size() - 1] = true;

  result = merger.merge(tables, true, valid);

  ASSERT_EQ(2u, result[0]->size());
  ASSERT_EQ(2u, result[0]->dictionaryAt(0)->size());
  ASSERT_EQ(2u, result[0]->dictionaryAt(1)->size());
  ASSERT_EQ(2u, result[0]->dictionaryAt(2)->size());

}

TEST_F(MergeTests, logarithmic_vs_simple_merge_test) {
  TableGenerator g(true);
  hyrise::storage::atable_ptr_t main = g.int_random(1000, 1);
  hyrise::storage::atable_ptr_t delta = g.int_random_delta(1000, 1);

  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger1(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_simple = merger1.merge(tables);

  TableMerger merger2(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_heap = merger2.merge(tables);

  ASSERT_TRUE(result_simple[0]->contentEquals(result_heap[0]));
}

TEST_F(MergeTests, row_wise_merger_vs_simple_merge_test) {
  TableGenerator g(true);
  hyrise::storage::atable_ptr_t main = g.int_random(1000, 3);
  hyrise::storage::atable_ptr_t delta = g.int_random_delta(1000, 3);

  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger1(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_simple = merger1.merge(tables);

  TableMerger merger2(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_heap = merger2.merge(tables);


  ASSERT_TRUE(result_simple[0]->contentEquals(result_heap[0]));
}


TEST_F(MergeTests, simple_merger_delta_test) {
  TableGenerator g(true);
  hyrise::storage::atable_ptr_t main1 = g.int_random(1000, 1);
  hyrise::storage::atable_ptr_t main2 = g.int_random(1000, 1);
  hyrise::storage::atable_ptr_t delta = g.create_empty_table_modifiable(1000, 1);

  // copy main2 into delta
  delta->resize(main2->size());
  for (size_t i = 0; i < main2->size(); ++i) {
    hyrise_int_t v = main2->getValue<hyrise_int_t>(0, i);
    delta->setValue<hyrise_int_t>(0, i, v);
  }

  // delta and main2 should be equal
  ASSERT_TRUE(delta->contentEquals(main2));


  std::vector<hyrise::storage::c_atable_ptr_t > tables1;
  tables1.push_back(main1);
  tables1.push_back(main2);

  std::vector<hyrise::storage::c_atable_ptr_t > tables2;
  tables2.push_back(main1);
  tables2.push_back(delta);

  TableMerger merger1(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_1 = merger1.merge(tables1);

  TableMerger merger2(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_2 = merger2.merge(tables2);

  ASSERT_TRUE(result_1[0]->contentEquals(result_2[0]));

}

TEST_F(MergeTests, sequential_heap_merger_delta_test) {
  TableGenerator g(true);
  hyrise::storage::atable_ptr_t main1 = g.int_random(10, 1);
  hyrise::storage::atable_ptr_t main2 = g.int_random(10, 1);
  hyrise::storage::atable_ptr_t delta = g.create_empty_table_modifiable(10, 1);

  // copy main2 into delta
  delta->resize(main2->size());
  for (size_t i = 0; i < main2->size(); ++i) {
    hyrise_int_t v = main2->getValue<hyrise_int_t>(0, i);
    delta->setValue<hyrise_int_t>(0, i, v);
  }

  // delta and main2 should be equal
  ASSERT_TRUE(delta->contentEquals(main2));

  std::vector<hyrise::storage::c_atable_ptr_t> tables1;
  tables1.push_back(main1);
  tables1.push_back(main2);

  std::vector<hyrise::storage::c_atable_ptr_t > tables2;
  tables2.push_back(main1);
  tables2.push_back(delta);

  TableMerger merger1(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_1 = merger1.merge(tables1);

  TableMerger merger2(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_2 = merger2.merge(tables2);

  ASSERT_TRUE(result_1[0]->contentEquals(result_2[0]));

}

TEST_F(MergeTests, DISABLED_parallel_heap_merger_delta_test) {
  TableGenerator g(true);
  hyrise::storage::atable_ptr_t main1 = g.int_random(1000, 1);
  hyrise::storage::atable_ptr_t main2 = g.int_random(1000, 1);
  hyrise::storage::atable_ptr_t delta = g.create_empty_table_modifiable(1000, 1);

  // copy main2 into delta
  delta->resize(main2->size());
  for (size_t i = 0; i < main2->size(); ++i) {
    hyrise_int_t v = main2->getValue<hyrise_int_t>(0, i);
    delta->setValue<hyrise_int_t>(0, i, v);
  }

  // delta and main2 should be equal
  ASSERT_TRUE(delta->contentEquals(main2));

  std::vector<hyrise::storage::c_atable_ptr_t > tables1;
  tables1.push_back(main1);
  tables1.push_back(main2);

  std::vector<hyrise::storage::c_atable_ptr_t> tables2;
  tables2.push_back(main1);
  tables2.push_back(delta);

  TableMerger merger1(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_1 = merger1.merge(tables1);

  TableMerger merger2(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result_2 = merger2.merge(tables2);

  ASSERT_TRUE(result_1[0]->contentEquals(result_2[0]));

}

TEST_F(MergeTests, simple_logarithmic_merger_test) {
  auto m = io::Loader::shortcuts::loadMainDelta("test/merge1_main.tbl", "test/merge1_delta.tbl");
  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(m->getMainTable());
  tables.push_back(m->getDeltaTable());
  TableMerger merger(new DefaultMergeStrategy(), new SequentialHeapMerger());
  const auto& result = merger.merge(tables);

  const auto& correct_result = io::Loader::shortcuts::load("test/merge1_result.tbl");
  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

}

TEST_F(MergeTests, DISABLED_parallel_value_merger_test) {
  auto m = io::Loader::shortcuts::loadMainDelta("test/merge1_main.tbl", "test/merge1_delta.tbl", io::Loader::params().setCompressed(false));
  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(m->getMainTable());
  tables.push_back(m->getDeltaTable());

  TableMerger merger(new DefaultMergeStrategy(), new SequentialHeapMerger());

  const auto& result = merger.merge(tables);

  const auto& correct_result = io::Loader::shortcuts::load("test/merge1_result.tbl");
  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

}

TEST_F(MergeTests, merge_with_different_layout) {
  auto main = io::Loader::shortcuts::load("test/merge1_main.tbl");
  auto delta = io::Loader::shortcuts::load("test/merge1_delta.tbl"); //, io::Loader::params().set_modifiable(true));
  auto correct_result = io::Loader::shortcuts::load("test/merge1_result.tbl");

 auto dest = io::Loader::shortcuts::load("test/merge1_newlayout.tbl", io::Loader::params().setModifiableMutableVerticalTable(true));

  ASSERT_EQ(3u, main->partitionCount());
  ASSERT_EQ(1u, dest->partitionCount());

  std::vector<hyrise::storage::c_atable_ptr_t> tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger(new DefaultMergeStrategy(), new SequentialHeapMerger());

  const auto& result = merger.mergeToTable(dest, tables);

  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

  ASSERT_EQ(1u, result[0]->partitionCount());
  ASSERT_EQ(dest, result[0]);

}

TEST_F(MergeTests, merge_with_different_layout_2) {
  auto main = io::Loader::shortcuts::load("test/merge1_main.tbl");
  auto delta = io::Loader::shortcuts::load("test/merge1_delta.tbl"); //, io::Loader::params().set_modifiable(true));
  auto correct_result = io::Loader::shortcuts::load("test/merge1_result.tbl");

  hyrise::storage::atable_ptr_t dest = io::Loader::shortcuts::load("test/merge1_newlayout_2.tbl", io::Loader::params().setModifiableMutableVerticalTable(true));

  ASSERT_EQ(3u, main->partitionCount());
  ASSERT_EQ(2u, dest->partitionCount());

  std::vector<hyrise::storage::c_atable_ptr_t> tables;
  tables.push_back(main);
  tables.push_back(delta);


  TableMerger merger(new DefaultMergeStrategy(), new SequentialHeapMerger());

  const auto& result = merger.mergeToTable(dest, tables);

  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

  ASSERT_EQ(2u, result[0]->partitionCount());
  ASSERT_EQ(dest, result[0]);
}


TEST_F(MergeTests, store_merge_compex) {
  auto linxxxs = std::dynamic_pointer_cast<storage::Store>(io::Loader::shortcuts::load("test/lin_xxxs.tbl"));
  auto ref = std::dynamic_pointer_cast<storage::Store>(io::Loader::shortcuts::load("test/reference/lin_xxxs_update.tbl"));
  linxxxs->resizeDelta(2);
  linxxxs->copyRowToDelta(linxxxs, 0, 0, 1);
  linxxxs->copyRowToDelta(linxxxs, 4, 1, 1);

  linxxxs->getDeltaTable()->setValue<hyrise_int_t>(1,0,99);
  linxxxs->getDeltaTable()->setValue<hyrise_int_t>(1,1,99);

  TableMerger merger(new DefaultMergeStrategy(), new SequentialHeapMerger());

  std::vector<hyrise::storage::c_atable_ptr_t> tables;
  tables.push_back(linxxxs->getMainTable());
  tables.push_back(linxxxs->getDeltaTable());

  // Valid Vector
  std::vector<bool> valid = {0,1,1,1,0,1,1};
  auto result = merger.merge(tables, true, valid);
  ASSERT_EQ(5u, result[0]->size());

  EXPECT_RELATION_EQ(ref, result[0]);
}

}}

