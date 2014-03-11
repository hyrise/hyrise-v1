// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <storage/Store.h>

namespace hyrise {
namespace storage {

class StoreRangeView : public Store {
 public:
  StoreRangeView(store_ptr_t table, size_t size = (size_t) -1, size_t offset = 0);
};

} } // namespace hyrise::storage

