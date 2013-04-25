// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "helper/HwlocHelper.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"
#include <hwloc.h>

namespace hyrise {
namespace access {

class HwLocHelperTest : public AccessTest {};

static void print_children(hwloc_topology_t topology, hwloc_obj_t obj,
                           int depth)
{
    char string[128];
    unsigned i;

    hwloc_obj_snprintf(string, sizeof(string), topology, obj, "#", 0);
    printf("%*s%s\n", 2*depth, "", string);
    for (i = 0; i < obj->arity; i++) {
        print_children(topology, obj->children[i], depth + 1);
    }
}
int test_hwloc(void)
{
    int depth;
    unsigned i;
    unsigned long size;
    int levels;
    char string[128];
    int topodepth;
    hwloc_topology_t topology;
    hwloc_obj_t obj;

    /* Allocate and initialize topology object. */
    hwloc_topology_init(&topology);

    /* ... Optionally, put detection configuration here to ignore
       some objects types, define a synthetic topology, etc....

       The default is to detect all the objects of the machine that
       the caller is allowed to access.  See Configure Topology
       Detection. */

    /* Perform the topology detection. */
    hwloc_topology_load(topology);

    /* Optionally, get some additional topology information
       in case we need the topology depth later. */
    topodepth = hwloc_topology_get_depth(topology);

    /*****************************************************************
     * First example:
     * Walk the topology with an array style, from level 0 (always
     * the system level) to the lowest level (always the proc level).
     *****************************************************************/
    for (depth = 0; depth < topodepth; depth++) {
        printf("*** Objects at level %d\n", depth);
        for (i = 0; i < hwloc_get_nbobjs_by_depth(topology, depth);
             i++) {
            hwloc_obj_snprintf(string, sizeof(string), topology,
                       hwloc_get_obj_by_depth(topology, depth, i),
                       "#", 0);
            printf("Index %u: %s\n", i, string);
        }
    }

    /*****************************************************************
     * Second example:
     * Walk the topology with a tree style.
     *****************************************************************/
    printf("*** Printing overall tree\n");
    print_children(topology, hwloc_get_root_obj(topology), 0);

    /*****************************************************************
     * Third example:
     * Print the number of sockets.
     *****************************************************************/
    depth = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
    if (depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
        printf("*** The number of sockets is unknown\n");
    } else {
        printf("*** %u socket(s)\n",
               hwloc_get_nbobjs_by_depth(topology, depth));
    }

    /*****************************************************************
     * Fourth example:
     * Compute the amount of cache that the first logical processor
     * has above it.
     *****************************************************************/
    levels = 0;
    size = 0;
    for (obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_PU, 0);
         obj;
         obj = obj->parent)
      if (obj->type == HWLOC_OBJ_CACHE) {
        levels++;
        size += obj->attr->cache.size;
      }
    printf("*** Logical processor 0 has %d caches totaling %luKB\n",
           levels, size / 1024);


    /* Destroy topology object. */
    hwloc_topology_destroy(topology);

    return 0;
}

// get number of cores on system and compare to sum of cores fopr each node
TEST_F(HwLocHelperTest, number_of_nodes){
  int cores = getNumberOfCoresOnSystem();
  int cores2 = 0;
  unsigned nodes = hwloc_get_nbobjs_by_type(getHWTopology(), HWLOC_OBJ_NODE);
  for(unsigned i = 0; i < nodes; i++){
    cores2 += getCoresForNode(getHWTopology(), i).size();
  }
  EXPECT_EQ(cores2, cores);
}

TEST_F(HwLocHelperTest, get_node_for_core){
  std::vector<unsigned> cores2;
  unsigned nodes = hwloc_get_nbobjs_by_type(getHWTopology(), HWLOC_OBJ_NODE);
  for(unsigned i = 0; i < nodes; i++){
    cores2 = getCoresForNode(getHWTopology(), i);
    for(size_t j = 0; j < cores2.size(); j++){
      EXPECT_EQ(i, getNodeForCore(cores2.at(j)));
    }
  }
}

}
}
