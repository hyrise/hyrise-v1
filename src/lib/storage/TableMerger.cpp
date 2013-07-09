// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/TableMerger.h>

#include "storage/AbstractMerger.h"


hyrise::storage::column_mapping_t identityMap(hyrise::storage::atable_ptr_t input) {
  hyrise::storage::column_mapping_t map;
  for (size_t column_index = 0; column_index < input->columnCount(); ++column_index)
    map[column_index] = column_index;
  return map;
}

std::vector<hyrise::storage::atable_ptr_t> TableMerger::mergeToTable(hyrise::storage::atable_ptr_t dest,
                                                                     std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                                                     bool useValid, std::vector<bool> valid) const {

  // Check that the valid vector has the right size
  assert(!useValid || (useValid && valid.size() == hyrise::functional::sum(input_tables, 0ul, [](hyrise::storage::c_atable_ptr_t& t){ return t->size(); })));

  // at least two tables
  if (input_tables.size() == 0) {
    throw std::runtime_error("Cannot call TableMerger with a less than two tables to merge");
  }

  merge_tables tables = _strategy->determineTablesToMerge(input_tables);
  assert(tables.tables_to_merge.size() >= 1);

  // Prepare modifiable output vector
  std::vector<hyrise::storage::atable_ptr_t> result;
  for(const auto& tab : tables.tables_not_to_merge)
    result.push_back(std::const_pointer_cast<AbstractTable>(tab));

  if (tables.tables_to_merge.size() > 0) {
    // copy metadata
    auto mapping = hyrise::storage::calculateMapping(tables.tables_to_merge.front(), dest);
    dest->setGeneration(tables.tables_to_merge.front()->generation() + 1);

    // do the merge
    auto newSize = _strategy->calculateNewSize(tables.tables_to_merge, useValid, valid);
    _merger->mergeValues(tables.tables_to_merge, dest, mapping, newSize, useValid, valid);

    // create result tables
    result.push_back(dest);
  }

  return result;
}

std::vector<hyrise::storage::atable_ptr_t > TableMerger::merge(std::vector<hyrise::storage::c_atable_ptr_t > &input_tables,
                                                                bool useValid, std::vector<bool> valid) const {
  hyrise::storage::atable_ptr_t merged_table;

  // Check that the valid vector has the right size
  assert(!useValid || (useValid && valid.size() == hyrise::functional::sum(input_tables, 0ul, [](hyrise::storage::c_atable_ptr_t& t){ return t->size(); })));

  // at least two tables
  if (input_tables.size() < 2) {
   throw std::runtime_error("Cannot call TableMerger with a less than two tables to merge");
  }

  merge_tables tables = _strategy->determineTablesToMerge(input_tables);
  assert(tables.tables_to_merge.size() != 1);

  // Prepare modifiable output vector
  std::vector<hyrise::storage::atable_ptr_t> result;
  for(const auto& tab : tables.tables_not_to_merge)
    result.push_back(std::const_pointer_cast<AbstractTable>(tab));

  if (tables.tables_to_merge.size() > 1) {

    // calculate new size - insert only
    size_t new_size = _strategy->calculateNewSize(tables.tables_to_merge, useValid, valid);

    // copy metadata
    merged_table = input_tables[0]->copy_structure(nullptr /*fields*/, false /*reuse dict*/, new_size, true /*with containers*/, _compress);
    merged_table->setGeneration(tables.tables_to_merge.front()->generation() + 1);

    // do the merge
    _merger->mergeValues(tables.tables_to_merge, merged_table, identityMap(merged_table), new_size, useValid, valid);

    // create result tables
    result.push_back(merged_table);
  }

  return result;
}

TableMerger *TableMerger::copy() {
  return new TableMerger(_strategy->copy(), _merger->copy());
}

