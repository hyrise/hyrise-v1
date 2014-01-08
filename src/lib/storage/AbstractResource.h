#pragma once

#include <helper/types.h>

namespace hyrise {
namespace storage {

/// Base class for all storable resources to
/// be used in plans.
class AbstractResource {
public:
  AbstractResource();
  virtual ~AbstractResource();

  resource_id_t id() const;

private:
  const resource_id_t _id;
};

} } // namespace hyrise::storage

