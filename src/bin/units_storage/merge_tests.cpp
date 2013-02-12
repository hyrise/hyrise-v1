// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <cstdlib>

#include <io.h>
#include <io/shortcuts.h>

#include <storage.h>

#include <helper/PapiTracer.h>
#include <helper/types.h>

class MergeTests : public ::hyrise::Test {};


TEST_F(MergeTests, simple_merge_test_with_rows) {
  hyrise::storage::atable_ptr_t main = Loader::shortcuts::load("test/merge1_main_row.tbl");
  hyrise::storage::atable_ptr_t delta = Loader::shortcuts::load("test/merge1_delta.tbl"); // oader::params().setModifiable(true));
  hyrise::storage::atable_ptr_t correct_result = Loader::shortcuts::load("test/merge1_result.tbl");

  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());

  const auto& result = merger.merge(tables);

  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);
}



TEST_F(MergeTests, simple_merge_test) {
  hyrise::storage::atable_ptr_t main = Loader::shortcuts::load("test/merge1_main.tbl");
  hyrise::storage::atable_ptr_t delta = Loader::shortcuts::load("test/merge1_delta.tbl");
  hyrise::storage::atable_ptr_t correct_result = Loader::shortcuts::load("test/merge1_result.tbl");

  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());

  const auto& result = merger.merge(tables);

  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

}

TEST_F(MergeTests, logarithmic_vs_simple_merge_test) {
  TableGenerator g(true);
  hyrise::storage::atable_ptr_t main = g.int_random(1000, 1);
  hyrise::storage::atable_ptr_t delta = g.int_random_delta(1000, 1);

  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger1(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
  const auto& result_simple = merger1.merge(tables);

  TableMerger merger2(new LogarithmicMergeStrategy(0), new SequentialHeapMergerRow());
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

  TableMerger merger1(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
  const auto& result_simple = merger1.merge(tables);

  TableMerger merger2(new LogarithmicMergeStrategy(0), new SequentialHeapMergerRow());
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

  TableMerger merger1(new LogarithmicMergeStrategy(0), new SequentialHeapMergerRow());
  const auto& result_1 = merger1.merge(tables1);

  TableMerger merger2(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
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

  TableMerger merger1(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
  const auto& result_1 = merger1.merge(tables1);

  TableMerger merger2(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
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

  TableMerger merger1(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
  const auto& result_1 = merger1.merge(tables1);

  TableMerger merger2(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
  const auto& result_2 = merger2.merge(tables2);

  ASSERT_TRUE(result_1[0]->contentEquals(result_2[0]));

}

TEST_F(MergeTests, simple_logarithmic_merger_test) {
  auto m = Loader::shortcuts::loadMainDelta("test/merge1_main.tbl", "test/merge1_delta.tbl");
  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(m->getMainTables()[0]);
  tables.push_back(m->getDeltaTable());
  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());
  const auto& result = merger.merge(tables);

  const auto& correct_result = Loader::shortcuts::load("test/merge1_result.tbl");
  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

}

TEST_F(MergeTests, DISABLED_parallel_value_merger_test) {
  auto m = Loader::shortcuts::loadMainDelta("test/merge1_main.tbl", "test/merge1_delta.tbl", Loader::params().setCompressed(false));
  std::vector<hyrise::storage::c_atable_ptr_t > tables;
  tables.push_back(m->getMainTables()[0]);
  tables.push_back(m->getDeltaTable());

  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());

  const auto& result = merger.merge(tables);

  const auto& correct_result = Loader::shortcuts::load("test/merge1_result.tbl");
  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

}

TEST_F(MergeTests, logarithmic_generation_1_test) {
  int runs = 16;
  int correct_generations[16] = {1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1};
  int actual_generations[16];

  std::vector<hyrise::storage::c_atable_ptr_t> tables({Loader::shortcuts::load("test/merge1_delta.tbl")});
  actual_generations[0] = 1;

  TableMerger merger(new LogarithmicMergeStrategy(1), new SequentialHeapMerger());
  for (int i = 1; i < runs; i++) {
    auto delta = Loader::shortcuts::load("test/merge1_delta.tbl");
    tables.push_back(delta);
    const auto& result = merger.merge(tables);
    actual_generations[i] = result.size();
    tables = std::vector<hyrise::storage::c_atable_ptr_t>(result.begin(), result.end());

  }

  ASSERT_TRUE(memcmp(correct_generations, actual_generations, sizeof(runs)*runs) == 0);
}

TEST_F(MergeTests, logarithmic_generation_2_test) {
  int runs = 16;
  int correct_generations[16] = {1, 2, 1, 2, 3, 2, 3, 4, 1, 2, 3, 2, 3, 4, 3, 4};
  int actual_generations[16];

 
  std::vector<hyrise::storage::c_atable_ptr_t> tables({Loader::shortcuts::load("test/merge1_delta.tbl")});
  actual_generations[0] = 1;

  TableMerger merger(new LogarithmicMergeStrategy(2), new SequentialHeapMerger());

  for (int i = 1; i < runs; i++) {
    hyrise::storage::atable_ptr_t delta = Loader::shortcuts::load("test/merge1_delta.tbl");
    tables.push_back(delta);
    const auto& result = merger.merge(tables);
    actual_generations[i] = result.size();
    tables = std::vector<hyrise::storage::c_atable_ptr_t>(result.begin(), result.end());
  }

  ASSERT_TRUE(memcmp(correct_generations, actual_generations, sizeof(runs)*runs) == 0);
}

TEST_F(MergeTests, merge_with_different_layout) {
  hyrise::storage::atable_ptr_t main = Loader::shortcuts::load("test/merge1_main.tbl");
  hyrise::storage::atable_ptr_t delta = Loader::shortcuts::load("test/merge1_delta.tbl"); //, Loader::params().set_modifiable(true));
  hyrise::storage::atable_ptr_t correct_result = Loader::shortcuts::load("test/merge1_result.tbl");

  hyrise::storage::atable_ptr_t dest = Loader::shortcuts::load("test/merge1_newlayout.tbl", Loader::params().setModifiableMutableVerticalTable(true));

  ASSERT_EQ(3u, main->sliceCount());
  ASSERT_EQ(1u, dest->sliceCount());

  std::vector<hyrise::storage::c_atable_ptr_t> tables;
  tables.push_back(main);
  tables.push_back(delta);

  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());

  const auto& result = merger.mergeToTable(dest, tables);

  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

  ASSERT_EQ(1u, result[0]->sliceCount());
  ASSERT_EQ(dest, result[0]);

}

TEST_F(MergeTests, merge_with_different_layout_2) {
  hyrise::storage::atable_ptr_t main = Loader::shortcuts::load("test/merge1_main.tbl");
  hyrise::storage::atable_ptr_t delta = Loader::shortcuts::load("test/merge1_delta.tbl"); //, Loader::params().set_modifiable(true));
  hyrise::storage::atable_ptr_t correct_result = Loader::shortcuts::load("test/merge1_result.tbl");

  hyrise::storage::atable_ptr_t dest = Loader::shortcuts::load("test/merge1_newlayout_2.tbl", Loader::params().setModifiableMutableVerticalTable(true));

  ASSERT_EQ(3u, main->sliceCount());
  ASSERT_EQ(2u, dest->sliceCount());

  std::vector<hyrise::storage::c_atable_ptr_t> tables;
  tables.push_back(main);
  tables.push_back(delta);


  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());

  const auto& result = merger.mergeToTable(dest, tables);

  ASSERT_TRUE(result[0]->contentEquals(correct_result));
  ASSERT_TRUE(result[0]->dictionaryAt(0)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(1)->size() == 7);
  ASSERT_TRUE(result[0]->dictionaryAt(2)->size() == 8);

  ASSERT_EQ(2u, result[0]->sliceCount());
  ASSERT_EQ(dest, result[0]);

}
