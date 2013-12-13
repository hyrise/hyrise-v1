// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "GroupValue.h"

#include "storage/Table.h"
#include "storage/meta_storage.h"
#include "storage/storage_types.h"
#include "storage/hash_functor.h"
#include "storage/storage_types_helper.h"

#include "boost/functional/hash.hpp"

namespace hyrise {
namespace storage {

u_int64_t GroupValue::hash_vids(const ValueIdList &vids) {
  u_int64_t h = FNV1_64_INIT;
  for(const ValueId& v: vids) {
    h = FNV_64A_INT(h, v.valueId);
    h = FNV_64A_INT(h, v.table);
  }
  return h;
}

size_t GroupValue::hash_value(atable_ptr_t source, size_t f, ValueId vid) {
  hash_functor<size_t> fun(source.get(), f, vid);
  type_switch<hyrise_basic_types> ts;
  return ts(source->typeOfColumn(f), fun);
}

size_t GroupValue::hash_group_values(atable_ptr_t source, const ValueIdList &vids, field_list_t &fields) {
  // FIXME: When we use 0 as seed value, we tend to get early hash collisions, a real implementation would need to fix this...
  size_t seed = 0x9e3779b9 / 2;

  for (unsigned i = 0; i < fields.size(); ++i) {
    boost::hash_combine(seed, hash_value(source, fields[i], vids[i]));
  }

  return seed;
}

} } // namespace hyrise::storage

