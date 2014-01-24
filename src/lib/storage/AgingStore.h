// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include <helper/types.h>
#include <storage/Store.h>

namespace hyrise {
namespace storage {

class AgingStore : public Store {
public:
  AgingStore() = delete;
  explicit AgingStore(const store_ptr_t& store);
  explicit AgingStore(const Store& store);
  virtual ~AgingStore();

  virtual void debugStructure(size_t level = 0) const;

private:
  atable_ptr_t makeAgingMain(const atable_ptr_t& table);
};

} } // namespace hyrise::storage

