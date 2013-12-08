// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "storage/AbstractTable.h"

namespace hyrise {
namespace io {

class AbstractColumnLoader {
 public:
  virtual ~AbstractColumnLoader() {}
};

template <typename T>
class ColumnLoader : public AbstractColumnLoader {
 private:
  std::vector<T>  _values;

 public:
  explicit ColumnLoader(size_t size = 0) {
    _values.reserve(size);
  }

  virtual ~ColumnLoader() {
  }

  void push_back(T value) {
    _values.push_back(value);
  }

  void write_to_table(storage::AbstractTable *table, size_t column) {

    if (_values.size() == 0) {
      return;
    }


    auto *dict = (storage::BaseDictionary<T> *)table->dictionaryAt(column);

    if (dict->isOrdered()) {
      // Create the sorted list for the input
      std::vector<T> *vec_sorted = new std::vector<T>(_values.begin(), _values.end());
      sort(vec_sorted->begin(), vec_sorted->end());

      T last_value = vec_sorted->at(0);
      dict->addValue(last_value);

      for (const auto& value: *vec_sorted) {
        if (value == last_value) {
          continue;
        }

        dict->addValue(value);
        last_value = value;
      }
      size_t row = 0;
      for (const auto& value: _values) {
        table->setValueId(column, row++, table->getValueIdForValue<T>(column, value));
      }

      delete vec_sorted;
    } else {
      size_t row = 0;
      for (const auto& value: _values) {
        table->setValue<T>(column, row++, value);
      }

    }
  }
};

} } // namespace hyrise::io

