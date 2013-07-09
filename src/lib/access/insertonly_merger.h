// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCES_DISCARDINGMERGER_H_
#define SRC_LIB_ACCES_DISCARDINGMERGER_H_

#include <vector>

#include "helper/types.h"
#include "storage/AbstractMerger.h"
#include "storage/TableUtils.h"

namespace hyrise { namespace insertonly {

/// Merger that removes all invisible elements from resulting table
class DiscardingMerger : public AbstractMerger {
 public:
  DiscardingMerger(tx::transaction_id_t txid) : _tx(txid) {};
  void mergeValues(const std::vector<storage::c_atable_ptr_t> &input_tables,
                   storage::atable_ptr_t merged_table,
                   const storage::column_mapping_t &column_mapping,
                   const uint64_t newSize,
                   bool useValid = false,
                   const std::vector<bool>& valid = std::vector<bool>());

  AbstractMerger* copy() { return nullptr; }
 private:
  tx::transaction_id_t _tx;
};

}}

#endif
