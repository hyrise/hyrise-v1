#include "unique_id.h"

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "boost/uuid/uuid_generators.hpp"

unique_id::unique_id(type val) : value(val) {
  static_assert(sizeof(unique_id::type) == sizeof(boost::uuids::uuid),
                "Sizes need to match for this wrapper to work");
}

unique_id unique_id::create() {
  auto uuid = boost::uuids::random_generator()();
  type value;
  std::copy(uuid.begin(), uuid.end(), value.begin());
  return unique_id(value);
}

namespace std {
string to_string(const unique_id& id) {
  const auto& as_uuid = *(reinterpret_cast<const boost::uuids::uuid*>(id.value.data()));
  return boost::uuids::to_string(as_uuid);
}

}

