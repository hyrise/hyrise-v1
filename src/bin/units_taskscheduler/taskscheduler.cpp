// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <iostream>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <sys/time.h>

#include "testing/test.h"

#include "access/NoOp.h"

#include "taskscheduler/SharedScheduler.h"
#include "taskscheduler/CoreBoundQueuesScheduler.h"
#include "taskscheduler/WSCoreBoundQueuesScheduler.h"
#include "taskscheduler/ThreadPerTaskScheduler.h"
#include "taskscheduler/DynamicPriorityScheduler.h"

#include "helper/HwlocHelper.h"


namespace hyrise {
namespace taskscheduler {

#if GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::ValuesIn;

// list schedulers to be tested
std::vector<std::string> getSchedulersToTest() {
 return {"WSCoreBoundQueuesScheduler",
           "CoreBoundQueuesScheduler",
           "CentralScheduler",
           "CentralPriorityScheduler",
           "CoreBoundPriorityQueuesScheduler",
           "WSCoreBoundPriorityQueuesScheduler",
           "ThreadPerTaskScheduler",
           "DynamicPriorityScheduler"};
}

class SchedulerTest : public TestWithParam<std::string> {
 public:
  virtual void SetUp() {
    scheduler_name = GetParam();
  }

 protected:
  std::string scheduler_name;
};

INSTANTIATE_TEST_CASE_P(
    Scheduler,
    SchedulerTest,
    ValuesIn(getSchedulersToTest()));

TEST_P(SchedulerTest, setScheduler) {
  SharedScheduler::getInstance().resetScheduler("CoreBoundQueuesScheduler");
  const auto& scheduler = SharedScheduler::getInstance().getScheduler();
  std::shared_ptr<CoreBoundQueuesScheduler> simple_task_scheduler = std::dynamic_pointer_cast<CoreBoundQueuesScheduler>(scheduler);
  bool test = (simple_task_scheduler == NULL);
  ASSERT_EQ(test, false);

  SharedScheduler::getInstance().resetScheduler(scheduler_name, getNumberOfCoresOnSystem());
}

TEST_P(SchedulerTest, wait_task_test) {
  SharedScheduler::getInstance().resetScheduler(scheduler_name);
  const auto& scheduler = SharedScheduler::getInstance().getScheduler();

  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();
  scheduler->schedule(waiter);
  waiter->wait();
}

long int getTimeInMillis() {
  /* Linux */
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  long int ret = tv.tv_usec;
  /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
  ret /= 1000;
  /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
  ret += (tv.tv_sec * 1000);
  return ret;
}



TEST_P(SchedulerTest, sync_task_test) {
  SharedScheduler::getInstance().resetScheduler(scheduler_name);
  const auto& scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(2);

  auto nop1 = std::make_shared<access::NoOp>();
  auto nop2 = std::make_shared<access::NoOp>();
  std::shared_ptr<SyncTask> syn = std::make_shared<SyncTask>();
  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();
  syn->addDependency(nop1);
  syn->addDependency(nop2);
  waiter->addDependency(syn);
  scheduler->schedule(nop1);
  scheduler->schedule(nop2);
  scheduler->schedule(syn);
  scheduler->schedule(waiter);
  waiter->wait();
}

TEST_P(SchedulerTest, million_dependencies_test) {
#ifdef EXPENSIVE_TESTS
  int tasks_group1 = 1000;
  int tasks_group2 = 1000;
  std::vector<std::shared_ptr<access::NoOp> > vtasks1;
  std::vector<std::shared_ptr<access::NoOp> > vtasks2;

  SharedScheduler::getInstance().resetScheduler(scheduler_name);
  const auto& scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(threads1);

  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();

  for (int i = 0; i < tasks_group1; ++i) {
    vtasks1.push_back(std::make_shared<access::NoOp>());
  }
  for (int i = 0; i < tasks_group2; ++i) {
    vtasks2.push_back(std::make_shared<access::NoOp>());
    for (int j = 0; j < tasks_group1; ++j) {
      vtasks2[i]->addDependency(vtasks1[j]);
    }
    waiter->addDependency(vtasks2[i]);
  }
  for (int i = 0; i < tasks_group1; ++i) {
    scheduler->schedule(vtasks1[i]);
  }
  for (int i = 0; i < tasks_group2; ++i) {
    scheduler->schedule(vtasks2[i]);
  }

  scheduler->schedule(waiter);
  waiter->wait();
#endif
}

TEST_P(SchedulerTest, million_noops_test) {
#ifdef EXPENSIVE_TESTS
  //chaned to 10.000, 1.000.000 takes too long on small computers
  int tasks_group1 = 10000;
  std::vector<std::shared_ptr<access::NoOp> > vtasks1;

  SharedScheduler::getInstance().resetScheduler(scheduler_name);
  const auto& scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(threads1);

  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();

  for (int i = 0; i < tasks_group1; ++i) {
    vtasks1.push_back(std::make_shared<access::NoOp>());
    waiter->addDependency(vtasks1[i]);
    scheduler->schedule(vtasks1[i]);
  }

  scheduler->schedule(waiter);
  waiter->wait();
#endif
}

TEST_P(SchedulerTest, wait_dependency_task_test) {
  SharedScheduler::getInstance().resetScheduler(scheduler_name);
  const auto& scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(2);
  auto nop = std::make_shared<access::NoOp>();
  auto waiter = std::make_shared<WaitTask>();
  waiter->addDependency(nop);
  scheduler->schedule(nop);
  scheduler->schedule(waiter);
  waiter->wait();
}

TEST_P(SchedulerTest, wait_set_test) {
  //int threads1 = 4;
  int tasks = 100;
  //in microseconds
  int sleeptime = 50;

  SharedScheduler::getInstance().resetScheduler(scheduler_name);
  const auto& scheduler = SharedScheduler::getInstance().getScheduler();

  auto waiter = std::make_shared<WaitTask>();
  auto sleeper = std::make_shared<SleepTask>(sleeptime);

  std::vector<std::shared_ptr<access::NoOp> > vtasks;
  for (int i = 0; i < tasks; ++i) {
    vtasks.push_back(std::make_shared<access::NoOp>());
    sleeper->addDependency(vtasks[i]);
    scheduler->schedule(vtasks[i]);
  }
  waiter->addDependency(sleeper);
  scheduler->schedule(sleeper);
  scheduler->schedule(waiter);
  waiter->wait();
}
#endif

bool long_block_test(AbstractTaskScheduler * scheduler){
    int threads1 = 2;

    int longSleepTasks = 1;
    int longSleepTime = 100000;
    int shortSleepTasks = 10;
    int shortSleepTime = 10000;
    int waittime = shortSleepTime;

    int upperLimit = longSleepTime / 1000 + (shortSleepTime * shortSleepTasks / (threads1 * 1000)) + waittime/1000;

    std::vector<std::shared_ptr<SleepTask> > longTasks;
    std::vector<std::shared_ptr<SleepTask> > shortTasks;
    std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();

    for (int i = 0; i < longSleepTasks; ++i) {
      longTasks.push_back(std::make_shared<SleepTask>(longSleepTime));
      waiter->addDependency(longTasks[i]);
    }

    for (int i = 0; i < shortSleepTasks; ++i) {
      shortTasks.push_back(std::make_shared<SleepTask>(shortSleepTime));
      waiter->addDependency(shortTasks[i]);
    }

    long int start, finish;
    start = getTimeInMillis();

    for (int i = 0; i < longSleepTasks; ++i) {
      scheduler->schedule(longTasks[i]);
    }

    // wait until long Task has started
    usleep(waittime);

    //usleep(shortSleepTime);
    for (int i = 0; i < shortSleepTasks; ++i) {
      scheduler->schedule(shortTasks[i]);
    }
    scheduler->schedule(waiter);
    waiter->wait();
    //TBD
    finish = getTimeInMillis();
    long int diff = finish - start;
    return (diff < upperLimit);
}

TEST(SchedulerBlockTest, dont_block_test) {
  /* we assign a long running task and a number of smaller tasks with a think time to the queues -
     the scheduler should realize that one queue is blocked and assign tasks to other queues */
  auto scheduler = std::make_shared<CoreBoundQueuesScheduler>(2);
  // These test currently just check for execute
  long_block_test(scheduler.get());
}

TEST(SchedulerBlockTest, dont_block_test_with_work_stealing) {
  /*  steal work from that queue */
  auto scheduler = std::make_shared<WSCoreBoundQueuesScheduler>(2);
  long_block_test(scheduler.get());
}

} } // namespace hyrise::taskscheduler

