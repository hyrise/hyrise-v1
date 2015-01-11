// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <storage/AbstractMergeStrategy.h>
#include <storage/AbstractMerger.h>
#include <storage/Store.h>

namespace hyrise {
namespace storage {

class ColumnStoreMerger {
 public:
  ColumnStoreMerger(std::shared_ptr<Store> store, bool forceFullIndexRebuild = false);
  void merge();
  template <typename T>
  void mergeDictionary(uint64_t column);

 private:
  void mergeValues(uint64_t column, atable_ptr_t newMain);
  std::shared_ptr<Store> _store;
  atable_ptr_t _main;
  atable_ptr_t _delta;
  std::vector<std::pair<std::shared_ptr<AbstractIndex>, std::vector<field_t>>> _main_indices, _delta_indices;

  size_t _newMainSize;
  size_t _columnCount;

  size_t _currentIndexToMerge;
  std::vector<value_id_t> _vidMappingMain;
  std::vector<value_id_t> _vidMappingDelta;

  std::vector<storage::atable_ptr_t> _newTables;

  bool _forceFullIndexRebuild;
};
}
}  // namespace hyrise::storage
