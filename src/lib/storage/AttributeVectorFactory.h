// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>

#include "helper/types.h"
#include "storage/AbstractDictionary.h"
#include "storage/BaseAttributeVector.h"

namespace hyrise {
namespace storage {

enum class CONCURRENCY_FLAG {
  CONCURRENT,
  NOT_CONCURRENT
};
enum class COMPRESSION_FLAG {
  UNCOMPRESSED,
  COMPRESSED
};

using baseattr_ptr_tr = std::shared_ptr<BaseAttributeVector<value_id_t>>;

baseattr_ptr_tr create_compressed_attribute_vector(size_t cols, size_t rows, std::vector<adict_ptr_t>& dicts);


/// Use this function to obtain an attribute vector
baseattr_ptr_tr create_attribute_vector(size_t cols,
                                        size_t rows,
                                        CONCURRENCY_FLAG concurrent,
                                        COMPRESSION_FLAG compressed,
                                        std::vector<adict_ptr_t>& dicts);
}
}  // namespace hyrise::storage
