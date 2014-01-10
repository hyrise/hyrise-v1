#pragma once

#include <memory>
#include <type_traits>

#include "storage/AbstractDictionary.h"
#include "storage/ColumnMetadata.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/OrderIndifferentDictionary.h"
#include "storage/ConcurrentUnorderedDictionary.h"
#include "storage/PassThroughDictionary.h"
#include "storage/meta_storage.h"

#include "boost/mpl/at.hpp"
#include "boost/mpl/int.hpp"
#include "boost/mpl/size.hpp"



namespace hyrise { namespace storage { namespace detail {

// Mapping types used to create the dictionaries
typedef boost::mpl::vector<OrderPreservingDictionary<hyrise_int_t>, OrderPreservingDictionary<hyrise_float_t>, OrderPreservingDictionary<hyrise_string_t>,
			   // Delta Types
			   OrderIndifferentDictionary<hyrise_int_t>, OrderIndifferentDictionary<hyrise_float_t>, OrderIndifferentDictionary<hyrise_string_t>,
			   // Concurrent Types
			   ConcurrentUnorderedDictionary<hyrise_int_t>, ConcurrentUnorderedDictionary<hyrise_float_t>, ConcurrentUnorderedDictionary<hyrise_string_t>,
			   // No Dict Types
			   PassThroughDictionary<hyrise_int32_t>, PassThroughDictionary<hyrise_float_t> > dictionary_mapping_types;


struct DictionaryCreatorDetail {
  using value_type = std::shared_ptr<AbstractDictionary>;
  std::size_t size;

  template<typename R>
  value_type operator()() {
    return std::make_shared<R>(size);
  }
};



} // namespace detail




// Create dictionary for type and size,
inline std::shared_ptr<AbstractDictionary> makeDictionary(DataType type, size_t size=0) {
  detail::DictionaryCreatorDetail dc {size};
  return type_switch<detail::dictionary_mapping_types>()(type, dc);
}


inline std::shared_ptr<AbstractDictionary> makeDictionary(const ColumnMetadata& c, size_t size=0) {
  return makeDictionary(c.getType(), size);
}



}}

