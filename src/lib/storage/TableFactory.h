// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_TABLEFACTORY_H_
#define SRC_LIB_STORAGE_TABLEFACTORY_H_

#include <memory>

#include "helper/types.h"

#include "storage/AbstractTable.h"
#include "storage/Table.h"

class AbstractTableFactory {
public:
  virtual ~AbstractTableFactory();
  virtual hyrise::storage::atable_ptr_t generate(std::vector<const ColumnMetadata*> *meta,
      std::vector<AbstractTable::SharedDictionaryPtr> *d = nullptr,
      size_t initial_size = 0,
      bool sorted = true,
      bool compressed = false) = 0;
};

class TableFactory : public AbstractTableFactory {
 public:
  hyrise::storage::atable_ptr_t generate(std::vector<const ColumnMetadata *> *m,
                                                 std::vector<AbstractTable::SharedDictionaryPtr> *d = nullptr,
                                                 size_t initial_size = 0,
                                                 bool sorted = true,
                                                 bool compressed = true) {
    return std::make_shared<Table>(m, d, initial_size, sorted, compressed);
  }
};

#endif  // SRC_LIB_STORAGE_TABLEFACTORY_H_
