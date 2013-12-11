// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include "storage/TableUtils.h"

namespace hyrise {
namespace storage {

class AbstractMerger {
public:
  virtual ~AbstractMerger() {}
  virtual void mergeValues(const std::vector<c_atable_ptr_t > &input_tables,
                           atable_ptr_t merged_table,
                           const column_mapping_t &column_mapping,
                           const uint64_t newSize,
                           bool useValid = false,
                           const std::vector<bool>& valid = std::vector<bool>()) = 0;
  virtual AbstractMerger *copy() = 0;
};

} } // namespace hyrise::storage

