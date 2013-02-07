#ifndef SRC_LIB_STORAGE_ABSTRACTALLOCATEDTABLE_H_
#define SRC_LIB_STORAGE_ABSTRACTALLOCATEDTABLE_H_

#include <memory/StrategizedAllocator.h>
#include <memory/MallocStrategy.h>
#include <memory/MemalignStrategy.h>

class AbstractTable;

template<typename Strategy, template <typename T, typename S> class Allocator, class AllocatedClass>
class AbstractAllocatedTable : public AbstractTable {
  typedef AbstractAllocatedTable<Strategy, Allocator, AllocatedClass> ALLOCATION_TEMPLATE;
public:

  virtual ~AbstractAllocatedTable() {};

  static void *operator new(size_t sz) {
    return Strategy::allocate(sz);
  }

  static void operator delete(void *p) {
    Strategy::deallocate(p, sizeof(AllocatedClass));
  }
};

//#define ALLOC_TEMPLATE template<typename Strategy = MemalignStrategy<4096>, template <typename T, typename S> class Allocator = StrategizedAllocator>
#define ALLOC_TEMPLATE template<typename Strategy = MallocStrategy, template <typename T, typename S> class Allocator = StrategizedAllocator>
#define ALLOC_FUNC_TEMPLATE template<typename Strategy, template <typename T, typename S> class Allocator>

#define ALLOC_INHERIT(arg) class arg : public AbstractAllocatedTable<Strategy, Allocator, arg<Strategy, Allocator> >

#define ALLOC_CLASS(arg) ALLOC_TEMPLATE ALLOC_INHERIT(arg)

#endif  // SRC_LIB_STORAGE_ABSTRACTALLOCATEDTABLE_H_

