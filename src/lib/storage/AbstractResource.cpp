#include "AbstractResource.h"

namespace hyrise {
namespace storage {

namespace {
  resource_id_t getUniqueId() {
    static resource_id_t id = 0;
    return ++id;
  }
}

AbstractResource::AbstractResource() :
  _id(getUniqueId()) {}

AbstractResource::~AbstractResource() {}

resource_id_t AbstractResource::id() const {
  return _id;
}

}
}  // namespace hyrise::storage
