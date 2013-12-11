// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>
#include <storage/AbstractTable.h>
#include <helper/vector_helpers.h>

namespace hyrise {
namespace storage {

typedef struct _merge_tables {

  _merge_tables(std::vector<c_atable_ptr_t > _tables_to_merge,
                std::vector<c_atable_ptr_t > _tables_not_to_merge) :
      tables_to_merge(_tables_to_merge), tables_not_to_merge(_tables_not_to_merge) {
  }

  const std::vector<c_atable_ptr_t > tables_to_merge;
  const std::vector<c_atable_ptr_t > tables_not_to_merge;
} merge_tables;


/*
*/
class AbstractMergeStrategy {
 public:
  AbstractMergeStrategy() {}
  virtual ~AbstractMergeStrategy() {}
  virtual merge_tables determineTablesToMerge(std::vector<c_atable_ptr_t > &input_tables) const = 0;
  virtual size_t calculateNewSize(const std::vector<c_atable_ptr_t > &input_tables, bool useValid, const std::vector<bool>& valid) const = 0;
  virtual AbstractMergeStrategy *copy() = 0;
};

class DefaultMergeStrategy : public AbstractMergeStrategy {

public:

  virtual merge_tables determineTablesToMerge(std::vector<c_atable_ptr_t > &input_tables) const {
    std::vector<c_atable_ptr_t > result;
    std::copy(std::begin(input_tables), std::end(input_tables), std::back_inserter(result));
    return _merge_tables(result, decltype(result)());
  }

  virtual size_t calculateNewSize(const std::vector<c_atable_ptr_t > &input_tables, bool useValid, const std::vector<bool>& valid) const {
    if (!useValid)
      return functional::sum(input_tables, 0u, [](c_atable_ptr_t& t){ return t->size(); });

    size_t parts = 0;
    return functional::sum(input_tables, 0u, [&parts,valid](c_atable_ptr_t& t){ 
      size_t newSize = 0; 
      for(size_t i=parts; i < parts + t->size(); ++i ) {
        if (valid[i]) ++newSize;
      }
      parts += t->size();
      return newSize;
    });
  }

  virtual AbstractMergeStrategy *copy() {
    throw std::runtime_error("Merge Strategy Copy is like killing a kitten");
  }

};

} } // namespace hyrise::storage

