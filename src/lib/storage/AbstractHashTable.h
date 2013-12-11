// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstdint>
#include <memory>


#include "helper/types.h"
#include "storage/AbstractResource.h"
#include "storage/storage_types.h"

namespace hyrise {
namespace storage {

class AbstractTable;

/// HashTable that maps table cells' hashed values of arbitrary columns to their rows.
class AbstractHashTable : public AbstractResource {
public:
  AbstractHashTable() {}

  virtual ~AbstractHashTable() {}

  /// Returns the number of key value pairs of underlying hash map structure.
  virtual size_t size() const = 0;

  /// Get positions for values in the table cells of given row and columns.
  virtual pos_list_t get(const c_atable_ptr_t& table,
                         const field_list_t &columns,
                         const pos_t row) const = 0;

  virtual c_atable_ptr_t getTable() const = 0;

  virtual field_list_t getFields() const = 0;

  virtual size_t getFieldCount() const = 0;

  virtual uint64_t numKeys() const = 0;
};

} } // namespace hyrise::storage


