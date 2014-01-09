// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>

#include "helper/types.h"

#include "storage/AbstractTable.h"
#include "storage/Table.h"

namespace hyrise {
namespace storage {

class AbstractTableFactory {
public:
  virtual ~AbstractTableFactory();
  virtual atable_ptr_t generate(std::vector<ColumnMetadata> *meta,
      std::vector<AbstractTable::SharedDictionaryPtr> *d = nullptr,
      size_t initial_size = 0,
      bool sorted = true,
      bool compressed = false) = 0;
};

class TableFactory : public AbstractTableFactory {
 public:
  atable_ptr_t generate(std::vector<ColumnMetadata> *m,
                                                 std::vector<AbstractTable::SharedDictionaryPtr> *d = nullptr,
                                                 size_t initial_size = 0,
                                                 bool sorted = true,
                                                 bool compressed = true) {
    return std::make_shared<Table>(m, d, initial_size, sorted, compressed);
  }
};

} } // namespace hyrise::storage

