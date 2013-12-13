#include "storage/HashTable.h"

#include "storage/meta_storage.h"
#include "storage/hash_functor.h"

namespace hyrise {
namespace storage {

size_t hash_value(const c_atable_ptr_t &source, const size_t &f, const ValueId &vid) {
  hash_functor<size_t> fun(source.get(), f, vid);
  type_switch<hyrise_basic_types> ts;
  return ts(source->typeOfColumn(f), fun);
}

} } // namespace hyrise::storage

