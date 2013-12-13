// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "AbstractMerger.h"

namespace hyrise { namespace storage {

class SimpleStoreMerger : public AbstractMerger {

  virtual ~SimpleStoreMerger(){}

  void mergeValues(const std::vector<c_atable_ptr_t> &input_tables,
                   atable_ptr_t merged_table,
                   const column_mapping_t &column_mapping,
                   const uint64_t newSize,
                   bool useValid = false,
                   const std::vector<bool>& valid = std::vector<bool>());
  
  AbstractMerger* copy() { return nullptr; }

};

}}

