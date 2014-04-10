#include "helper.h"

#include <random>
#include <vector>

#include "helper/types.h"
#include "storage/AbstractTable.h"
#include "storage/DictionaryFactory.h"
#include "storage/Table.h"
#include "storage/MutableVerticalTable.h"
#include "storage/FixedLengthVector.h"

using namespace hyrise;

namespace {
std::random_device rd;
std::mt19937 gen(rd());
}

storage::atable_ptr_t int_random_table(const size_t rows, const size_t cols, std::vector<size_t> partitions) {
  if (partitions.size() == 0) {
    partitions.resize(cols);
    std::fill(partitions.begin(), partitions.end(), 1);
  }
  if (std::accumulate(partitions.begin(), partitions.end(), 0u) != cols) {
    std::cout << cols << " " << partitions.size() << std::endl;
    throw std::runtime_error("Layout mismatch.");
  }
  std::vector<storage::atable_ptr_t> tables;
  size_t column_count = 0;
  for (size_t parts : partitions) {
    std::vector<storage::ColumnMetadata> meta;
    auto av = std::make_shared<storage::FixedLengthVector<value_id_t>>(/*cols*/ 1, rows);
    std::vector<storage::adict_ptr_t> dicts;
    for (size_t col = 0; col < parts; ++col) {
      meta.emplace_back("attr" + std::to_string(column_count), IntegerType);
      auto dict =
          std::dynamic_pointer_cast<storage::BaseDictionary<hyrise_int_t>>(storage::makeDictionary(IntegerType));
      for (hyrise_int_t value = 0; value < rows; ++value) {
        dict->addValue(value);  // value range is 0 to rows
      }
      std::uniform_int_distribution<> dis(0, rows - 1);
      for (size_t row = 0; row < rows; ++row) {
        av->set(0, row, dis(gen));
      }
      dicts.push_back(dict);
      column_count++;
    }
    tables.push_back(std::make_shared<storage::Table>(meta, av, dicts));
  }
  return std::make_shared<storage::MutableVerticalTable>(tables);
}

storage::atable_ptr_t empty_table(const size_t rows, const size_t cols, std::vector<size_t> partitions) {
  if (partitions.size() == 0) {
    partitions.resize(cols);
    std::fill(partitions.begin(), partitions.end(), 1);
  }
  if (std::accumulate(partitions.begin(), partitions.end(), 0u) != cols) {
    std::cout << cols << " " << partitions.size() << std::endl;
    throw std::runtime_error("Layout mismatch.");
  }
  std::vector<storage::atable_ptr_t> tables;
  size_t column_count = 0;
  for (size_t parts : partitions) {
    std::vector<storage::ColumnMetadata> meta;
    auto av = std::make_shared<storage::FixedLengthVector<value_id_t>>(/*cols*/ 1, rows);
    std::vector<storage::adict_ptr_t> dicts;
    for (size_t col = 0; col < parts; ++col) {
      meta.emplace_back("attr" + std::to_string(column_count), IntegerType);
      auto dict =
          std::dynamic_pointer_cast<storage::BaseDictionary<hyrise_int_t>>(storage::makeDictionary(IntegerTypeDelta));
      dicts.push_back(dict);
      column_count++;
    }
    tables.push_back(std::make_shared<storage::Table>(meta, av, dicts));
  }
  return std::make_shared<storage::MutableVerticalTable>(tables);
}
