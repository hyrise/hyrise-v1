// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_ABSTRACTALLOCATEDDICTIONARY_H_
#define SRC_LIB_STORAGE_ABSTRACTALLOCATEDDICTIONARY_H_

template<typename T>
class BaseDictionary;

template <
  typename DictionaryType, // type for dictionary
  class Strategy, // Allocation Strategy
  template<typename T, typename S> class Allocator, // Allocator
  class AllocatedClass // Allocated Size for size
  >
class BaseAllocatedDictionary : public BaseDictionary<DictionaryType> {
 public:

  virtual ~BaseAllocatedDictionary() {}; // When the destructor is virtual, static operators behave like virtual

  hyrise::storage::dict_ptr_t copy_empty() { return std::make_shared<AllocatedClass>(); }
  
  void *operator new(size_t sz) {
    return Strategy::allocate(sz);
  }

  void operator delete(void *p) {
    Strategy::deallocate(p, sizeof(AllocatedClass));
  }

};


#endif  // SRC_LIB_STORAGE_ABSTRACTALLOCATEDDICTIONARY_H_
