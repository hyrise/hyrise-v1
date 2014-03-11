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

  void age(const pos_list_t& posList);
  void age(field_t field, const pos_list_t& posList);

  size_t hotSize(const std::vector<storage::field_t>& fields = std::vector<storage::field_t>()) const;

private:
  atable_ptr_t makeAgingMain(const atable_ptr_t& table);
};

} } // namespace hyrise::storage

