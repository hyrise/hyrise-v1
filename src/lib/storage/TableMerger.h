// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>
#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <storage/AbstractMergeStrategy.h>
#include <storage/AbstractMerger.h>

namespace hyrise {
namespace storage {

class TableMerger {
public:

  TableMerger(AbstractMergeStrategy *strategy, AbstractMerger *merger, const bool compress = true) : _strategy(strategy), _merger(merger), _compress(compress) {}
  ~TableMerger() {
    delete _strategy;
    delete _merger;
  }

  std::vector<atable_ptr_t> merge(std::vector<c_atable_ptr_t> &input_tables, bool useValid = false, std::vector<bool> valid=std::vector<bool>()) const;

  /*
    This method allows to specify directly a table that is the
    result table and will contain the result of the merge
    process. This allows to modify the layout of the result table
    without interfering with the merge algorithms itself.
  */
  std::vector<atable_ptr_t > mergeToTable(atable_ptr_t dest, std::vector<c_atable_ptr_t> &input_tables, bool useValid = false, std::vector<bool> valid=std::vector<bool>()) const;

  TableMerger *copy();

private:
  AbstractMergeStrategy *_strategy;
  AbstractMerger *_merger;
  const bool _compress;
};

} } // namespace hyrise::storage
