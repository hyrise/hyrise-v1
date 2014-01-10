// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <storage/storage_types.h>

#include <string>
#include <stdexcept>

namespace hyrise {
namespace storage {

class ColumnMetaCreationException : public std::runtime_error {
 public:
  explicit ColumnMetaCreationException(const std::string &what) : std::runtime_error(what) {}
};

class ColumnMetadata {
 protected:
  std::string name;

  DataType type;

 public:
  static ColumnMetadata metadataFromString(std::string typestring, std::string name = "");

  ColumnMetadata(std::string n, DataType t) : name(n), type(t) { }

  ColumnMetadata() {}

  DataType getType() const {
    return type;
  }

  void setType(DataType t) {
    type = t;
  }

  std::string getName() const {
    return name;
  }

  void setName(std::string new_name) {
    this->name = new_name;
  }

  size_t getWidth() const {
    // TODO add actual width field
    return sizeof(value_id_t);
  }

  ColumnMetadata *copy() {
    return new ColumnMetadata(name, type);
  }

  bool matches(const ColumnMetadata& col) const {
    return (col.getName() == name) && (col.getType() == type);
  }
};

} } // namespace hyrise::storage

