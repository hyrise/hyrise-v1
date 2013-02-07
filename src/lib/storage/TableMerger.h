// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_STORAGE_TABLEMERGER_H_
#define SRC_LIB_STORAGE_TABLEMERGER_H_

#include <vector>
#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <storage/AbstractMergeStrategy.h>
#include <storage/AbstractMerger.h>

class TableMerger {
public:

  TableMerger(AbstractMergeStrategy *strategy, AbstractMerger *merger, const bool compress = true) : _strategy(strategy), _merger(merger), _compress(compress) {}
  ~TableMerger() {
    delete _strategy;
    delete _merger;
  }

  std::vector<hyrise::storage::atable_ptr_t> merge(std::vector<hyrise::storage::c_atable_ptr_t> &input_tables) const;

  /*
    This method allows to specify directly a table that is the
    result table and will contain the result of the merge
    process. This allows to modify the layout of the result table
    without interfering with the merge algorithms itself.
  */
  std::vector<hyrise::storage::atable_ptr_t > mergeToTable(hyrise::storage::atable_ptr_t dest, std::vector<hyrise::storage::c_atable_ptr_t> &input_tables) const;

  TableMerger *copy();

private:
  AbstractMergeStrategy *_strategy;
  AbstractMerger *_merger;
  const bool _compress;
};

#endif  // SRC_LIB_STORAGE_TABLEMERGER_H_
