// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_COLUMNPROPERTIES_H_
#define SRC_LIB_STORAGE_COLUMNPROPERTIES_H_

#include <cstdlib>
#include <vector>
#include <string>


enum ColumnType {
  ColDefaultType = 0,
  // TODO: BitCompressed, FixedLength ...
  ColDefaultDictVector
};

// struct ColumnProperty - compressed, sorted, ...


// store properties of multiple columns
// Use default values, if types are not set.
class ColumnProperties {
public:
  ColumnProperties();
  virtual ~ColumnProperties();

  ColumnType defaultType() const;
  void setDefaultType(ColumnType type);

  void setType(size_t column, ColumnType type);
  ColumnType getType(size_t column) const;

  static ColumnType typeFromString(const std::string& typeName);

protected:
  ColumnType _defaultType;
  std::vector<ColumnType> _types;
};


#endif  // SRC_LIB_STORAGE_COLUMNPROPERTIES_H_