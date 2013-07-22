#include "storage/HashTable.h"

#include "storage/meta_storage.h"
#include "storage/hash_functor.h"

size_t hash_value(const hyrise::storage::c_atable_ptr_t &source, const size_t &f, const ValueId &vid) {
  hyrise::storage::hash_functor<size_t> fun(source.get(), f, vid);
  hyrise::storage::type_switch<hyrise_basic_types> ts;
  return ts(source->typeOfColumn(f), fun);
}
