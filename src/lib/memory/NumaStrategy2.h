// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_MEMORY_NUMASTRATEGY2_H_
#define SRC_LIB_MEMORY_NUMASTRATEGY2_H_

#include "NumaStrategy.h"

#include <errno.h>
#include <hwloc.h>
#include <unistd.h>
#include <string>

template<typename config>
class NumaNodeStrategy2 {

 public:

#ifdef WITH_NUMA

  static hwloc_topology_t getTopology() {
    static hwloc_topology_t *topology = nullptr;
    if (topology == nullptr) {
      topology = (hwloc_topology_t *) malloc(sizeof(hwloc_topology_t));
      hwloc_topology_init(topology);
      hwloc_topology_load(*topology);
    }
    return *topology;
  }


  static void *allocate(const size_t sz) {
    void *res = nullptr;
    int a_res = posix_memalign(&res, getpagesize(), sz);

    if (a_res != 0)
      throw std::bad_alloc();

    hwloc_topology_t topology = getTopology();
    hwloc_obj_t obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NODE, config::node);
    if (obj != nullptr) { // found a numa group
      int alloc_result = hwloc_set_area_membind(topology,
                                                res, sz, obj->nodeset,
                                                HWLOC_MEMBIND_BIND,
                                                HWLOC_MEMBIND_NOCPUBIND | HWLOC_MEMBIND_STRICT | HWLOC_MEMBIND_MIGRATE);
      if (res == nullptr || alloc_result < 0)
        throw std::runtime_error(strerror(errno));
    }
    return res;
  }


  static void *reallocate(void *old, size_t sz, size_t old_sz) {
    void *new_data = realloc(old, sz);
    hwloc_topology_t topology = getTopology();
    hwloc_obj_t obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NODE, config::node);
    if (obj != nullptr) { // found a numa group
      int alloc_result = hwloc_set_area_membind(topology,
                                                new_data, sz, obj->nodeset,
                                                HWLOC_MEMBIND_BIND,
                                                HWLOC_MEMBIND_NOCPUBIND | HWLOC_MEMBIND_STRICT | HWLOC_MEMBIND_MIGRATE);
      if (alloc_result < 0)
        throw std::runtime_error(strerror(errno));

      // Return new pointer
    }
    return new_data;
  }

  static void deallocate(void *p, size_t sz) {
    hwloc_topology_t topology;
    hwloc_topology_init(&topology);
    hwloc_free(topology, p, sz);
  }

#else

  static void *allocate(size_t sz) {
    return malloc(sz);
  };

  static void *reallocate(void *old, size_t sz, size_t /*old_sz*/) {
    return realloc(old, sz);
  };

  static void deallocate(void *p, size_t sz) {
    free(p);
  };

#endif //WITH_NUMA
};


#endif  // SRC_LIB_MEMORY_NUMASTRATEGY2_H_
