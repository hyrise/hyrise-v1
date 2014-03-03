// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "testing/test.h"
#include "taskscheduler/Task.h"

namespace hyrise {
namespace taskscheduler {

class FooTask : public Task {
  const std::string vname() override { return "FooTask"; }
};

class BarTask : public Task {
  const std::string vname() override { return "BarTask"; }
};

class MixedDependencies : public ::testing::Test {
 protected:
  void SetUp() override {
    // The foo tasks
    t1 = std::make_shared<FooTask>();
    t2 = std::make_shared<FooTask>();
    t3 = std::make_shared<FooTask>();
    // The bar task
    t4 = std::make_shared<BarTask>();
    t5 = std::make_shared<BarTask>();

    // t1 has t2, t3, and t4 as successors
    t2->addDependency(t1);
    t3->addDependency(t1);
    t4->addDependency(t1);

    t2->addDependency(t5);
  }

  std::shared_ptr<Task> t1, t2, t3, t4, t5;
};


TEST_F(MixedDependencies, returnsAllFooSuccessors) { EXPECT_EQ(2, t1->getAllSuccessorsOf<FooTask>().size()); }

TEST_F(MixedDependencies, returnsAllBarSuccessors) { EXPECT_EQ(1, t1->getAllSuccessorsOf<BarTask>().size()); }

TEST_F(MixedDependencies, returnsAllTaskSuccessors) { EXPECT_EQ(3, t1->getAllSuccessorsOf<Task>().size()); }

TEST_F(MixedDependencies, returnsNoSuccessors) { EXPECT_EQ(0, t2->getAllSuccessorsOf<Task>().size()); }

TEST_F(MixedDependencies, returnsSinglePredecessor) { EXPECT_EQ(t1, t2->getFirstPredecessorOf<FooTask>()); }

TEST_F(MixedDependencies, noPredecessors) { ASSERT_ANY_THROW(t1->getFirstPredecessorOf<Task>()); }

// If multiple predecessors are found, return the first that was assigned.
TEST_F(MixedDependencies, multiplePredecessors) { EXPECT_EQ(t1, t2->getFirstPredecessorOf<Task>()); }
}
}
