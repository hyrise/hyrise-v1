// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "ColumnProperties.h"

ColumnProperties::ColumnProperties() : 
  _defaultType(ColDefaultType),
  _types(std::vector<ColumnType>())
{ }

ColumnProperties::~ColumnProperties()
{ }

ColumnType ColumnProperties::typeFromString(const std::string& typeName) {
  if (typeName.compare("ColDefaultDictVector") == 0)
    return ColDefaultDictVector;
  if (typeName.compare("ColDefaultType") == 0)
    return ColDefaultType;

  return ColInvalidType;
}

ColumnType ColumnProperties::defaultType() const {
  return _defaultType;
}

void ColumnProperties::setDefaultType(ColumnType type) {
  _defaultType = type;
}

void ColumnProperties::setType(size_t column, ColumnType type) {
  // expand types vector, if needed
  if (_types.size() <= column)
    _types.resize(column+1, _defaultType);

  _types[column] = type;
}

ColumnType ColumnProperties::getType(size_t column) const {
  // return default value if type is not set for column
  if (_types.size() <= column)
    return _defaultType;

  return _types[column];
}

