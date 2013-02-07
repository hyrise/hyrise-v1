#ifndef SRC_LIB_MEMORY_MEMALIGNSTRATEGY_H_
#define SRC_LIB_MEMORY_MEMALIGNSTRATEGY_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

template<int Alignment>
class MemalignStrategy {
 public:
  static void *allocate(size_t sz) {
    void *raw;
    int ret = posix_memalign(&raw, Alignment, sz);

    if (ret == 0) {
      return raw;
    }

    if (ret == ENOMEM) {
      printf("%s", "Error allocating memory");
    }

    if (ret == EINVAL) {
      printf("%s", "invalid alignment");
    }

    return nullptr;
  };

  static void *reallocate(void *old, size_t sz, size_t old_sz) {
    //Todo: Assure allignment
    return realloc(old, sz);
  }


  static void deallocate(void *p, size_t sz) {
    free(p);
  };
};
#endif  // SRC_LIB_MEMORY_MEMALIGNSTRATEGY_H_
