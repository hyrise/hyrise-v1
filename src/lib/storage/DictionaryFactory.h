#pragma once

#include "storage/AbstractDictionaryFactory.h"
#include "storage/TableDefinition.h"

namespace hyrise { namespace storage {

template <template <typename T> class D>
class DictFactory : public AbstractDictionaryFactory {
 public:
  std::shared_ptr<AbstractDictionary> create(const ColumnDefinition& cg) override {
    return AbstractDictionary::dictionaryWithType<DictionaryFactory<D>>(cg._type);
  }
};

class ExistingDictionary : public AbstractDictionaryFactory {
  std::shared_ptr<AbstractDictionary> _dict;
 public:
  ExistingDictionary(std::shared_ptr<AbstractDictionary> d) : _dict(d) {}
  std::size_t size() const { return _dict->size(); }
  std::shared_ptr<AbstractDictionary> create(const ColumnDefinition& cg) override {
    return _dict;
  }
};

}}
