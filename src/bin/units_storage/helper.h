// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_BIN_UNITS_STORAGE_HELPER_H_
#define SRC_BIN_UNITS_STORAGE_HELPER_H_

#include <gtest/gtest.h>
#include <storage/storage_types.h>
#include <storage/Table.h>
#include <storage/HashTable.h>

#include <algorithm>

// a random int table generator
hyrise::storage::atable_ptr_t int_random_table(const size_t rows,
                                               const size_t cols,
                                               std::vector<size_t> partitions = {});
// a modifiable empty table generator
hyrise::storage::atable_ptr_t empty_table(const size_t rows, const size_t cols, std::vector<size_t> partitions = {});

template <typename HT>
::testing::AssertionResult AssertHashTableFully(hyrise::storage::atable_ptr_t table, const field_list_t& columns) {
  HT ht(table, columns);
  if (testHashTableFullCoverage(ht, table, columns)) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << "The HashTable did not map the table correctly!";
  }
}

template <typename HT>
bool testHashTableFullCoverage(const HT& hashTable, hyrise::storage::atable_ptr_t table, const field_list_t& columns) {
  bool result = true;
  for (pos_t row = 0; row < table->size(); ++row) {
    pos_list_t positions = hashTable.get(table, columns, row);
    if (positions.empty()) {
      result = false;
      break;
    }
  }
  return result;
}
#endif  // SRC_BIN_UNITS_STORAGE_HELPER_H_
