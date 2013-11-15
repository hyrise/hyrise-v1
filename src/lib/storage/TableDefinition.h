#pragma once

#include <memory>
#include <string>
#include <vector>

#include "storage/BaseAttributeVector.h"
#include "storage/AbstractDictionary.h"

namespace hyrise { namespace storage {

class ColumnDefinition {
 public:
  DataType _type;
  std::string _name;
  std::unique_ptr<AbstractDictionaryFactory> _dictionary_factory;
  ColumnDefinition(DataType type,
                   std::string name,
                   std::unique_ptr<AbstractDictionaryFactory> df);
};

class TableDefinition {
 public:
  std::size_t _size = 0;
  std::unique_ptr<AbstractAttributeVectorFactory> _attribute_vector_factory;
  std::vector<ColumnDefinition> _columns;
  TableDefinition(std::size_t size,
                  std::unique_ptr<AbstractAttributeVectorFactory> af,
                  std::vector<ColumnDefinition> cols);

};

}}
