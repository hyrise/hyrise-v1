#pragma once

#include <memory>
#include "storage/AbstractDictionary.h"

namespace hyrise { namespace storage {

class ColumnDefinition;

class AbstractDictionaryFactory {
 public:
  virtual ~AbstractDictionaryFactory() {};
  virtual std::size_t size() const { return 0; }
  virtual std::shared_ptr< AbstractDictionary > create(const ColumnDefinition&) = 0;
};

}}
