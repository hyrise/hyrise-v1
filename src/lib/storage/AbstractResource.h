#ifndef SRC_LIB_STORAGE_ABSTRACTRESOURCE_H_
#define SRC_LIB_STORAGE_ABSTRACTRESOURCE_H_

/// Base class for all storable resources to
/// be used in plans.
class AbstractResource {
 public:
  virtual ~AbstractResource() {}

  virtual bool isTable() { return false; }
  virtual bool isIndex() { return false; }
};

#endif
