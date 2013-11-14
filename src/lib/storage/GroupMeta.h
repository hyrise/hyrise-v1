#pragma once

//#include <memory>

#include "helper/vector_helpers.h"
#include "storage/AbstractDictionary.h"
#include "storage/OrderIndifferentDictionary.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/FixedLengthVector.h"
#include "storage/BitCompressedVector.h"

namespace hyrise { namespace storage {

class TableMetaGenerator;

class ColumnGenerator;

class AbstractDictionaryFactory {
 public:
  virtual ~AbstractDictionaryFactory() {}
  virtual std::size_t size() const { return 0; }
  virtual std::shared_ptr< AbstractDictionary > create(ColumnGenerator&) = 0;
};

class AbstractAttributeVectorFactory {
 public:
  virtual ~AbstractAttributeVectorFactory() {}
  virtual std::shared_ptr< BaseAttributeVector<value_id_t> > create(TableMetaGenerator&) = 0;
};

class ColumnGenerator {
 public:
  DataType _type;
  std::string _name;
  std::unique_ptr<AbstractDictionaryFactory> _dictionary_factory;
  ColumnGenerator(DataType type, std::string name, std::unique_ptr<AbstractDictionaryFactory> df) : _type(type), _name(name), _dictionary_factory(std::move(df)) {}
};

class TableMetaGenerator {
 public:
  std::size_t _size = 0;
  std::unique_ptr<AbstractAttributeVectorFactory> _attribute_vector_factory;
  std::vector<ColumnGenerator> _columns;
  TableMetaGenerator(std::size_t size,
                     std::unique_ptr<AbstractAttributeVectorFactory> af,
                     std::vector<ColumnGenerator> cols) :
      _size(size), _attribute_vector_factory(std::move(af)), _columns(std::move(cols)) {}

};

template <template <typename T> class D>
class DictFactory : public AbstractDictionaryFactory {
 public:
  std::shared_ptr<AbstractDictionary> create(ColumnGenerator& cg) override {
    return AbstractDictionary::dictionaryWithType<DictionaryFactory<D>>(cg._type);
  }
};

class ExistingDictionary : public AbstractDictionaryFactory {
  std::shared_ptr<AbstractDictionary> _dict;
 public:
  ExistingDictionary(std::shared_ptr<AbstractDictionary> d) : _dict(d) {}
  std::size_t size() const { return _dict->size(); }
  std::shared_ptr<AbstractDictionary> create(ColumnGenerator& cg) override {
    return _dict;
  }
};

using OrderIndifferentDictionaryFactory = DictFactory<OrderIndifferentDictionary>;
using OrderPreservingDictionaryFactory = DictFactory<OrderPreservingDictionary>;

class FixedLengthVectorFactory : public AbstractAttributeVectorFactory {
 public:
  virtual std::shared_ptr<BaseAttributeVector<value_id_t> > create(TableMetaGenerator& t) {
    return std::make_shared<FixedLengthVector<value_id_t> >(t._columns.size(), t._size);
  }
};

class BitCompressedVectorFactory : public AbstractAttributeVectorFactory {
 public:
  virtual std::shared_ptr<BaseAttributeVector<value_id_t>> create(TableMetaGenerator& t) {
    std::vector<std::uint64_t> bits = functional::collect(t._columns, [] (const ColumnGenerator& c) {
        auto sz =  c._dictionary_factory->size();
        return sz == 1 ? 1 : std::uint64_t(ceil(log(sz) / log(2.0)));
      });
    return std::make_shared<BitCompressedVector<value_id_t>>(t._columns.size(), t._size, bits);
  }
};

}}
