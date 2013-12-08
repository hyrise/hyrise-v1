// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "taskscheduler/SharedScheduler.h"
#include "taskscheduler/Task.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"
#include "access/NoOp.h"
#include <memory>
#include <queue>
#include "helper.h"


namespace hyrise {
namespace access {

class PriorityTaskTest : public AccessTest {};

// test data distribution method applied to PlanOp input
TEST_F(PriorityTaskTest, compare_test){
  auto t1 = std::make_shared<NoOp>();
  auto t2 = std::make_shared<NoOp>();
  auto t3 = std::make_shared<NoOp>();
  t1->setPriority(1);
  t2->setPriority(2);
  t3->setPriority(3);

  std::priority_queue<std::shared_ptr<taskscheduler::Task>, std::vector<std::shared_ptr<taskscheduler::Task>>, taskscheduler::CompareTaskPtr> _runQueue;

  _runQueue.push(t1);
  _runQueue.push(t3);
  _runQueue.push(t2);

  EXPECT_EQ(t1, _runQueue.top());
}

}
}
