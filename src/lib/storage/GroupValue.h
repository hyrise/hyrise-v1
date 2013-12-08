// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "helper/hash.h"
#include <storage/storage_types.h>
#include <memory>

namespace hyrise {
namespace storage {

class AbstractTable;

class GroupValue {
  ValueIdList vids;
  u_int64_t hashv;
 public:
  typedef u_int64_t hash_t;

  GroupValue():
      vids(),
      hashv(0)
  { }

  explicit GroupValue(ValueIdList v):
      vids(v),
      hashv(0)
  { }

  void addValueId(ValueId v) {
    vids.push_back(v);
  }

  ValueId getValueId(size_t at) const {
    return vids[at];
  }

  u_int64_t hash_vids(const ValueIdList &vids);

  /*
    The goal of this hashing function is to be based on values instead of valueids
  */
  static size_t hash_value(std::shared_ptr<AbstractTable> source, size_t f, ValueId vid);

  // Given a table, the value ids and the fields it will hash the complete set
  static size_t hash_group_values(std::shared_ptr<AbstractTable> source, const ValueIdList &vids,
                                  field_list_t &fields);

  /*
    Uses the Hash function FNV
  */
  inline hash_t hash() {
    if (hashv != 0) {
      return hashv;
    }
    hashv = hash_vids(vids);
    return hashv;
  }

  inline bool operator==(GroupValue &other) {
    return hash() == other.hash();
  }
};

} } // namespace hyrise::storage

