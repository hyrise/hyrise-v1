// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_SIMPLESTOREMERGER_H_
#define SRC_LIB_STORAGE_SIMPLESTOREMERGER_H_

#include "AbstractMerger.h"

namespace hyrise { namespace storage {

class SimpleStoreMerger : public AbstractMerger {

  virtual ~SimpleStoreMerger(){}

  void mergeValues(const std::vector<hyrise::storage::c_atable_ptr_t> &input_tables,
                   hyrise::storage::atable_ptr_t merged_table,
                   const column_mapping_t &column_mapping,
                   const uint64_t newSize,
                   bool useValid = false,
                   const std::vector<bool>& valid = std::vector<bool>());
  
  AbstractMerger* copy() { return nullptr; }

};

}}
#endif // SRC_LIB_STORAGE_SIMPLESTOREMERGER_H_
