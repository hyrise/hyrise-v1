#pragma once

#include <memory>
#include <string>
#include <vector>

#include "storage/AbstractDictionaryFactory.h"
#include "storage/AbstractAttributeVectorFactory.h"

namespace hyrise { namespace storage {

class AbstractDictionaryFactory;
class AbstractAttributeVectorFactory;

class ColumnDefinition {
 public:
  DataType _type;
  std::string _name;
  std::unique_ptr<AbstractDictionaryFactory> _dictionary_factory;
  ColumnDefinition(DataType type,
                   std::string name,
                   std::unique_ptr<AbstractDictionaryFactory> df) :
      _type(type), _name(name), _dictionary_factory(std::move(df)) {}
};

class TableDefinition {
 public:
  std::size_t _size = 0;
  std::unique_ptr<AbstractAttributeVectorFactory> _attribute_vector_factory;
  std::vector<ColumnDefinition> _columns;
  TableDefinition(std::size_t size,
                  std::unique_ptr<AbstractAttributeVectorFactory> af,
                  std::vector<ColumnDefinition> cols) :
      _size(size), _attribute_vector_factory(std::move(af)), _columns(std::move(cols)) {}

};

}}
