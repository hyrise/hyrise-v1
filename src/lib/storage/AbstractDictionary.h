// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_ABSTRACTDICTIONARY_H_
#define SRC_LIB_STORAGE_ABSTRACTDICTIONARY_H_

#include <vector>
#include <string>
#include <memory>

#include <memory/StrategizedAllocator.h>
#include <memory/MallocStrategy.h>
#include <memory/MemalignStrategy.h>
#include <storage/storage_types.h>

class AbstractDictionary;

template < template<typename T, typename S, template<typename A, typename K> class A> class D,
         class Strategy = MallocStrategy,
         template<typename A, typename K> class Allocator = StrategizedAllocator >
struct DictionaryFactory {
  static std::shared_ptr<AbstractDictionary> build(DataType type, size_t size = 0) {
    switch (type) {
      case IntegerType:
        return std::make_shared<D<hyrise_int_t, Strategy, Allocator>>(size);
        break;

      case FloatType:
        return std::make_shared<D<hyrise_float_t, Strategy, Allocator>>(size);
        break;

      case StringType:
        return std::make_shared<D<hyrise_string_t, Strategy, Allocator>>(size);
        break;

      default:
        throw std::runtime_error("Type not supported for dictionary");
    }
  }
};


class AbstractDictionary {

public:

  virtual ~AbstractDictionary() {}

  virtual bool isOrdered() = 0;

  template< class Factory >
  static std::shared_ptr<AbstractDictionary> dictionaryWithType(DataType type, size_t size = 0) {
    return Factory::build(type, size);
  }

  virtual void reserve(size_t size) = 0;

  virtual std::shared_ptr<AbstractDictionary> copy() = 0;
  virtual std::shared_ptr<AbstractDictionary> copy_empty() = 0;
  virtual size_t size() = 0;

  virtual void shrink() = 0;

};

#endif  // SRC_LIB_STORAGE_ABSTRACTDICTIONARY_H_

