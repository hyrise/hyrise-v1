// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_STORAGE_COLUMNMETADATA_H_
#define SRC_LIB_STORAGE_COLUMNMETADATA_H_

#include <storage/storage_types.h>

#include <string>
#include <stdexcept>

class ColumnMetaCreationException : public std::runtime_error {
 public:
  explicit ColumnMetaCreationException(const std::string &what) : std::runtime_error(what) {}
};

class ColumnMetadata {
 protected:
  std::string name;

  DataType type;

 public:
  static ColumnMetadata *metadataFromString(std::string typestring, std::string name = "");

  ColumnMetadata(std::string n, DataType t) : name(n), type(t) { }

  ColumnMetadata() {}

  DataType getType() const {
    return type;
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

  bool matches(const ColumnMetadata *col) const {
    return (col->getName() == name) && (col->getType() == type);
  }
};

#endif  // SRC_LIB_STORAGE_COLUMNMETADATA_H_

