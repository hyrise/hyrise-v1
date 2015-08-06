// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_OPERATIONDATA_H_
#define SRC_LIB_ACCESS_OPERATIONDATA_H_

#include <vector>

#include "helper/types.h"
#include "storage/storage_types.h"

typedef std::vector<hyrise::storage::c_atable_ptr_t> table_list_t;
typedef std::vector<hyrise::storage::c_ahashtable_ptr_t> hash_table_list_t;

namespace hyrise {
namespace access {

/// Data container for operator input/output handling.
class OperationData {
 public:
  typedef std::vector<storage::c_aresource_ptr_t> aresource_vec_t;

  /// Adds a new resource
  void addResource(const storage::c_aresource_ptr_t& resource);

  /// Returns the n-th abstract resource
  storage::c_aresource_ptr_t getResource(size_t index) const;

  /// Return all resources
  const aresource_vec_t& all() const;

  /// Returns the n-th element of type T
  template <typename T>
  std::shared_ptr<const T> nthOf(size_t index) const;

  /// Sets the n-th element of type T
  /// \param index index with allOf<T>
  /// \param element is expected to be a shared_ptr<const T>
  template <typename T>
  void setNthOf(size_t index, const std::shared_ptr<const T>& element);

  /// Returns a vector of resources of type type T
  template <typename T>
  std::vector<std::shared_ptr<const T>> allOf() const;

  /// Returns number of passed resources of type T
  template <typename T>
  size_t sizeOf() const;

  /// Overall number of passed resources
  size_t size() const;

  /// Merge all elements of all member lists of given other objects into
  /// own ones, including retain counting.
  void mergeWith(OperationData& other);

  /// \defgroup deprecatedAccess Deprecated accessors
  /// The following accessors are mostly present as remnants of an old
  /// design. The use of the templates of OperationData-Impl.h is strongly
  /// encouraged. They merely wrap around the accessor template functions
  /// above.
  /// @{

  void addHash(storage::c_ahashtable_ptr_t input);
  void add(storage::c_atable_ptr_t input);
  void setHash(storage::c_ahashtable_ptr_t input, size_t index);
  void setTable(storage::c_atable_ptr_t input, size_t index);

  storage::c_atable_ptr_t getTable(const size_t index = 0) const;
  storage::c_ahashtable_ptr_t getHashTable(const size_t index = 0) const;

  size_t numberOfTables() const;
  size_t numberOfHashTables() const;
  bool emptyTables() const;
  bool emptyHashTables() const;

  table_list_t getTables() const;
  hash_table_list_t getHashTables() const;
  /// @}
 private:
  typedef std::runtime_error OperationDataError;
  aresource_vec_t _resources;
};
}
}

#endif  // SRC_LIB_ACCESS_OPERATIONDATA_H_
