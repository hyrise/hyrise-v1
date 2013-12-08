#pragma once

#include <memory>

#include "storage/AbstractDictionary.h"
#include "storage/meta_storage.h"

namespace hyrise { namespace storage { namespace detail {

template <template <typename T> class D>
struct DictionaryCreator {
  using value_type = std::shared_ptr<AbstractDictionary>;
  std::size_t size;

  template <typename R>
  std::shared_ptr<AbstractDictionary> operator()() {
    return std::make_shared<D<R>>(size);
  }
};

} // namespace detail

// Create dictionary for type and size,
// Usage: makeDictionary<OrderIndifferentDictionary>(IntegerType, 10),
// which returns shared_ptr<OrderIndifferentDictionary<hyrise_int_t>> (init-ed
// for 10 elements)
template <template <typename T> class D>
std::shared_ptr<AbstractDictionary> makeDictionary(DataType type, size_t size=0) {
  detail::DictionaryCreator<D> dc {size};
  return type_switch<hyrise_basic_types>()(type, dc);
}

}}

