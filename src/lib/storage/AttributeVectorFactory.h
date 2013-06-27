// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_ATTRIBUTEVECTORFACTORY_H_
#define SRC_LIB_STORAGE_ATTRIBUTEVECTORFACTORY_H_

#include <memory>

#include <storage/BaseAttributeVector.h>
#include <storage/FixedLengthVector.h>
#include <storage/DefaultDictVector.hpp>
#include <storage/BitCompressedVector.h>

class AttributeVectorFactory {
public:

  template <typename T, typename Allocator>
  static std::shared_ptr<BaseAttributeVector<T>> getAttributeVector(size_t columns = 1,
      size_t rows = 0,
      int distinct_values = 1,
      bool compressed = false,
      bool isDefaultDictVector = false) {

    if (isDefaultDictVector) {
      return std::make_shared<DefaultDictVector<T, Allocator> >(columns, rows);
    }

    return std::make_shared<FixedLengthVector<T, Allocator> >(columns, rows);
  }

  template <typename T, typename Allocator>
  static std::shared_ptr<BaseAttributeVector<T>> getAttributeVector2(size_t columns,
      size_t rows,
      bool compressed = false,
  std::vector<uint64_t> bits = std::vector<uint64_t> {}) {
    if (!compressed) {
      return std::make_shared<FixedLengthVector<T, Allocator> >(columns, rows);
    } else {
      return std::make_shared<BitCompressedVector<T, Allocator> >(columns, rows, bits);
    }

  }
};

#endif  // SRC_LIB_STORAGE_ATTRIBUTEVECTORFACTORY_H_
