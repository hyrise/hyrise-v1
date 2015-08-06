// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <storage/AbstractMergeStrategy.h>
#include <storage/AbstractMerger.h>
#include <storage/Store.h>
#include "optional.hpp"

namespace hyrise {
namespace storage {

struct sortPermutationHelper {
  pos_t position;
  bool fromMain;
};

class ColumnStoreMerger {
 public:
  ColumnStoreMerger(std::shared_ptr<Store> store, bool forceFullIndexRebuild = false, std::string sortIndexName = "");
  void merge();
  template <typename T>
  void mergeDictionary(uint64_t column);
  template <typename T>
  void mergeDictionarySorted(uint64_t column);

 private:
  void mergeValuesSorted(uint64_t column, atable_ptr_t newMain);
  void mergeValues(uint64_t column, atable_ptr_t newMain, bool indexMaintenance);
  size_t mergeValuesMain(size_t column, atable_ptr_t table);
  std::vector<struct sortPermutationHelper> calculatePermutations(size_t column);
  std::shared_ptr<Store> _store;
  atable_ptr_t _main;
  atable_ptr_t _delta;
  std::vector<std::pair<std::shared_ptr<AbstractIndex>, std::vector<field_t>>> _main_indices;

  size_t _newMainSize;
  size_t _columnCount;

  size_t _currentIndexToMerge;
  std::vector<value_id_t> _vidMappingMain;
  std::vector<value_id_t> _vidMappingDelta;

  std::vector<std::vector<value_id_t>> _mainDictMappings;
  std::vector<std::vector<value_id_t>> _deltaDictMappings;

  std::vector<storage::atable_ptr_t> _newTables;

  bool _forceFullIndexRebuild;
  std::optional<std::pair<std::shared_ptr<AbstractIndex>, std::vector<field_t>>> _sortIndex;

  std::vector<struct sortPermutationHelper> sortPermutations;
};
}
}  // namespace hyrise::storage
