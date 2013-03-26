// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_TABLEFACTORY_H_
#define SRC_LIB_STORAGE_TABLEFACTORY_H_

#include <memory>

#include "helper/types.h"

#include "AbstractTable.h"
#include "Table.h"

class AbstractTableFactory {
public:
  AbstractTableFactory();
  virtual ~AbstractTableFactory();
  virtual hyrise::storage::atable_ptr_t generate(std::vector<const ColumnMetadata*> *meta,
      std::vector<AbstractTable::SharedDictionaryPtr> *d = nullptr,
      size_t initial_size = 0,
      bool sorted = true,
      bool compressed = false,
      size_t padding_size = STORAGE_ALIGNMENT_SIZE,
      size_t _align_size = STORAGE_ALIGNMENT_SIZE,
      bool isDefaultDictVector = false) = 0;
};

/**
 * Specific subclass of the abstract factory
 */
template<typename Strategy = MallocStrategy, template <typename T, typename S> class Allocator = StrategizedAllocator>
class TableFactory : public AbstractTableFactory {
public:
  TableFactory();
  virtual ~TableFactory();
  virtual hyrise::storage::atable_ptr_t generate(std::vector<const ColumnMetadata *> *m,
      std::vector<AbstractTable::SharedDictionaryPtr> *d = nullptr,
      size_t initial_size = 0,
      bool sorted = true,
      bool compressed = true,
      size_t padding_size = STORAGE_ALIGNMENT_SIZE,
      size_t _align_size = STORAGE_ALIGNMENT_SIZE,
      bool isDefaultDictVector = false);
};

template<typename Strategy, template <typename T, typename S> class Allocator>
TableFactory<Strategy, Allocator>::TableFactory() {}

template<typename Strategy, template <typename T, typename S> class Allocator>
TableFactory<Strategy, Allocator>::~TableFactory() {}

template<typename Strategy, template <typename T, typename S> class Allocator>
hyrise::storage::atable_ptr_t TableFactory<Strategy, Allocator>::generate(std::vector<const ColumnMetadata *> *m,
    std::vector<AbstractTable::SharedDictionaryPtr> *d,
    size_t initial_size,
    bool sorted,
    bool compressed,
    size_t padding_size,
    size_t _align_size,
    bool isDefaultDictVector) {
  return std::make_shared<Table<Strategy, Allocator>>(m, d, initial_size, sorted, padding_size, _align_size, compressed, isDefaultDictVector);
}

#endif  // SRC_LIB_STORAGE_TABLEFACTORY_H_
