#include "ColumnProperties.h"

ColumnProperties::ColumnProperties() : 
  defaultType(ColDefaultType),
  _types(std::vector<ColumnType>())
{
}

ColumnProperties::~ColumnProperties()
{ 
}

ColumnType ColumnProperties::typeFromString(const std::string& typeName)
{
  if (typeName.compare("ColDefaultDictVector") == 0)
    return ColDefaultDictVector;

  return ColDefaultType;
}

void ColumnProperties::setType(size_t column, ColumnType type) {
  // expand types vector, if needed
  if (_types.size() <= column)
    _types.resize(column+1, defaultType);

  _types[column] = type;
}

ColumnType ColumnProperties::getType(size_t column) {
  // return default value if type is not set for column
  if (_types.size() <= column)
    return defaultType;

  return _types[column];
}

