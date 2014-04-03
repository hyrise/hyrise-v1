#ifndef SRC_LIB_ACCESS_OPERATIONDATA_IMPL_H_
#define SRC_LIB_ACCESS_OPERATIONDATA_IMPL_H_

#include <algorithm>
#include "access/system/OperationData.h"

namespace hyrise {
namespace access {

template <typename T>
std::shared_ptr<const T> OperationData::nthOf(size_t index) const {
  auto i = 0u;
  for (const auto& resource : _resources) {
    if (const auto& cast_resource = std::dynamic_pointer_cast<const T>(resource)) {
      if (i == index) {
        return cast_resource;
      }
      ++i;
    }
  }
  throw OperationDataError("Could not find resource indexed " + std::to_string(index) + " when only " +
                           std::to_string(i) + " available");
}

template <typename T>
void OperationData::setNthOf(size_t index, const std::shared_ptr<const T>& element) {
  auto i = 0u;
  size_t actual = 0;
  for (const auto& resource : _resources) {
    if (std::dynamic_pointer_cast<const T>(resource)) {
      if (i == index) {
        _resources[actual] = element;
        return;
      }
      ++i;
    }
    ++actual;
  }
  throw OperationDataError("Could not set resource indexed " + std::to_string(index) + " when only " +
                           std::to_string(i) + " available");
}

template <typename T>
std::vector<std::shared_ptr<const T>> OperationData::allOf() const {
  std::vector<std::shared_ptr<const T>> result;
  for (const auto& resource : _resources) {  // replace with copy_if
    if (const auto& cast_resource = std::dynamic_pointer_cast<const T>(resource)) {
      result.push_back(cast_resource);
    }
  }
  return result;
}

template <typename T>
size_t OperationData::sizeOf() const {
  typedef decltype(*std::begin(_resources)) element_type;
  return std::count_if(std::begin(_resources), std::end(_resources), [](const element_type& e) {
    return std::dynamic_pointer_cast<const T>(e) != nullptr;
  });
}
}
}

#endif
