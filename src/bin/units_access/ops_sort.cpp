// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <string>

#include "helper.h"

#include <access.h>
#include <storage.h>

#include "io/shortcuts.h"
#include "io/loaders.h"

namespace hyrise {
namespace access {

class SortTest : public AccessTest {};

TEST_F(SortTest, simple_sort_test) {
  auto data = io::Loader::shortcuts::load("test/reference/group_by_scan_using_table_2.tbl");
  const auto& reference = io::Loader::shortcuts::load("test/sort_test.tbl");

  SortScan s2;
  s2.addInput(data);
  s2.setSortField(0);
  auto ref2 = s2.execute()->getResultTable();

  ASSERT_TRUE(reference->contentEquals(ref2));

}

// TODO: Loader should be able to differentiate modifiable/nonmodifiable
TEST_F(SortTest, DISABLED_simple_sort_test_modifiable) {
  io::Loader::params p;
  p.setHeader(io::CSVHeader("test/reference/group_by_scan_using_table_2.tbl"));
  p.setInput(io::CSVInput("test/reference/group_by_scan_using_table_2.tbl"));
  p.setModifiableMutableVerticalTable(true);

  auto data = io::Loader::load(p);
  const auto& reference = io::Loader::shortcuts::load("test/sort_test.tbl");

  SortScan s2;
  s2.addInput(data);
  s2.setSortField(0);
  auto ref2 = s2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(reference, ref2);

}

TEST_F(SortTest, simple_sort_test_modifiable_unsorted) {
  const auto& reference = io::Loader::shortcuts::load("test/reference/unsorted_sort.tbl");
  auto data = reference->copy_structure_modifiable();

  data->resize(3);
  data->setValue<hyrise_int_t>(0, 0, 30);
  data->setValue<hyrise_int_t>(0, 1, 20);
  data->setValue<hyrise_int_t>(0, 2, 10);

  SortScan s2;
  s2.addInput(data);
  s2.setSortField(0);
  auto ref2 = s2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(reference, ref2);

}

}
}

