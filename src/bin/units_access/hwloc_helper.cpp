// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "helper/HwlocHelper.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"
#include <hwloc.h>

namespace hyrise {
namespace access {

class HwLocHelperTest : public AccessTest {};

// get number of cores on system and compare to sum of cores fopr each node
TEST_F(HwLocHelperTest, number_of_nodes) {
  int cores = getNumberOfCoresOnSystem();
  int cores2 = 0;
  unsigned nodes = hwloc_get_nbobjs_by_type(getHWTopology(), HWLOC_OBJ_NODE);
  if (nodes) {
    for (unsigned i = 0; i < nodes; i++) {
      cores2 += getCoresForNode(getHWTopology(), i).size();
    }
    EXPECT_EQ(cores2, cores);
  }
}

TEST_F(HwLocHelperTest, get_node_for_core) {
  std::vector<unsigned> cores2;
  unsigned nodes = hwloc_get_nbobjs_by_type(getHWTopology(), HWLOC_OBJ_NODE);
  for (unsigned i = 0; i < nodes; i++) {
    cores2 = getCoresForNode(getHWTopology(), i);
    for (size_t j = 0; j < cores2.size(); j++) {
      EXPECT_EQ(i, getNodeForCore(cores2.at(j)));
    }
  }
}
}
}
