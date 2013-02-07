// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_STORAGE_ABSTRACTMERGESTRATEGY_H_
#define SRC_LIB_STORAGE_ABSTRACTMERGESTRATEGY_H_

#include <vector>
#include <storage/AbstractTable.h>

typedef struct _merge_tables {

  _merge_tables(std::vector<hyrise::storage::c_atable_ptr_t > _tables_to_merge,
                std::vector<hyrise::storage::c_atable_ptr_t > _tables_not_to_merge) :
      tables_to_merge(_tables_to_merge), tables_not_to_merge(_tables_not_to_merge) {
  }

  const std::vector<hyrise::storage::c_atable_ptr_t > tables_to_merge;
  const std::vector<hyrise::storage::c_atable_ptr_t > tables_not_to_merge;
} merge_tables;

class AbstractMergeStrategy {
 public:
  AbstractMergeStrategy() {}
  virtual ~AbstractMergeStrategy() {}
  virtual merge_tables determineTablesToMerge(std::vector<hyrise::storage::c_atable_ptr_t > &input_tables) const = 0;
  virtual size_t calculateNewSize(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables) const = 0;
  virtual AbstractMergeStrategy *copy() = 0;
};

#endif  // SRC_LIB_STORAGE_ABSTRACTMERGESTRATEGY_H_
