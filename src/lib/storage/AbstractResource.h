#pragma once

namespace hyrise {
namespace storage {

/// Base class for all storable resources to
/// be used in plans.
class AbstractResource {
 public:
  virtual ~AbstractResource();
};

} } // namespace hyrise::storage

