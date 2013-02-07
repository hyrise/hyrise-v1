#ifndef SRC_LIB_STORAGE_BASEALLOCATEDATTRIBUTEVECTOR_H_
#define SRC_LIB_STORAGE_BASEALLOCATEDATTRIBUTEVECTOR_H_

#include "BaseAttributeVector.h"
#include "stdio.h"

template<typename AllocatedClass, typename Allocator>
class BaseAllocatedAttributeVector : public BaseAttributeVector<typename Allocator::value_type> {
  typedef BaseAllocatedAttributeVector<AllocatedClass, Allocator> ALLOCATION_TEMPLATE;
 public:
  virtual ~BaseAllocatedAttributeVector() {};

  static void *operator new(size_t sz) {
    return Allocator::Strategy::allocate(sz);
  }

  static void operator delete(void *p) {
    Allocator::Strategy::deallocate(p, sizeof(AllocatedClass));
  }
};


#endif  // SRC_LIB_STORAGE_BASEALLOCATEDATTRIBUTEVECTOR_H_
