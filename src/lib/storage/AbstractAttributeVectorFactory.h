#pragma once

#include "storage/BaseAttributeVector.h"

namespace hyrise { namespace storage {

class TableDefinition;
class AbstractAttributeVectorFactory {
 public:
  virtual ~AbstractAttributeVectorFactory() {}
  virtual std::shared_ptr< BaseAttributeVector<value_id_t> > create(const TableDefinition&) = 0;
};

}}
