// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "taskscheduler/NumaNodeWSCoreBoundTaskQueue.h"
#include "taskscheduler/NumaNodeWSTaskScheduler.h"
#include "access/NoOp.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"
#include "helper/HwlocHelper.h"

namespace hyrise {
namespace access {

class NumaTaskSchedulerTest : public AccessTest {};

TEST_F(NumaTaskSchedulerTest, number_of_numa_nodes){
  NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue> scheduler;
  EXPECT_EQ(scheduler.getNumaNodes(), getNumberOfNodes(getHWTopology()));
}


TEST_F(NumaTaskSchedulerTest, next_numa_node){
  unsigned nextcore;
  unsigned node = 1;
  unsigned numberOfCoresPerNode = getNumberOfCoresPerNumaNode();
  NumaNodeWSTaskScheduler<NumaNodeWSCoreBoundTaskQueue> scheduler;
  std::shared_ptr<NoOp> nop1 = std::make_shared<NoOp>();
  nop1->setPreferredNode(node);
  nextcore = scheduler.nextQueueForNode(node);
  for(unsigned i = 0; i < numberOfCoresPerNode; i++)
    scheduler.schedule(nop1);
  EXPECT_EQ(nextcore, scheduler.nextQueueForNode(node));
}

}
}
