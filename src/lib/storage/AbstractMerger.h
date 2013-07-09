// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_ABSTRACTMERGER_H_
#define SRC_LIB_STORAGE_ABSTRACTMERGER_H_

#include <vector>
#include <unordered_map>

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include "storage/TableUtils.h"

class AbstractMerger {
public:
  virtual ~AbstractMerger() {}
  virtual void mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                           hyrise::storage::atable_ptr_t merged_table,
                           const hyrise::storage::column_mapping_t &column_mapping,
                           const uint64_t newSize,
                           bool useValid = false,
                           const std::vector<bool>& valid = std::vector<bool>()) = 0;
  virtual AbstractMerger *copy() = 0;
};

#endif  // SRC_LIB_STORAGE_ABSTRACTMERGER_H_
