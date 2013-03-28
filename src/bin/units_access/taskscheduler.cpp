// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <iostream>
#include <algorithm>
#include <iterator>

#include "testing/test.h"
#include "helper.h"

#include "access.h"
#include "access/NoOp.h"
#include "access/TaskSchedulerAdjustment.h"
#include "access/PlanOperation.h"
#include "io/TransactionManager.h"
#include "taskscheduler.h"
#include <ctime>
#include <sys/time.h>
#include "helper/HwlocHelper.h"


namespace hyrise {

#if GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::ValuesIn;

// list schedulers to be tested
std::vector<std::string> getSchedulersToTest() {
  std::vector<std::string> result;
  result.push_back("WSSimpleTaskScheduler");
  result.push_back("SimpleTaskScheduler");
  result.push_back("CentralScheduler");
  return result;
}

class SchedulerTest : public TestWithParam<std::string> {
 public:
  SchedulerTest() {
    sm = StorageManager::getInstance();
  }
  virtual ~SchedulerTest() { }

  virtual void SetUp() {
    scheduler_name = GetParam();
    sm->removeAll(); // Make sure old tables don't bleed into these tests
    tx::TransactionManager::getInstance().reset();
  }

  virtual void TearDown() {
    scheduler_name = "";
  }

 protected:
  std::string scheduler_name;
  StorageManager *sm;
};

INSTANTIATE_TEST_CASE_P(
    Scheduler,
    SchedulerTest,
    ValuesIn(getSchedulersToTest()));

TEST_P(SchedulerTest, setScheduler) {
  SharedScheduler::getInstance().resetScheduler("SimpleTaskScheduler");
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();
  SimpleTaskScheduler<CoreBoundTaskQueue> * simple_task_scheduler = dynamic_cast<SimpleTaskScheduler<CoreBoundTaskQueue> *>(scheduler);
  bool test = (simple_task_scheduler == NULL);
  ASSERT_EQ(test, false);

  SharedScheduler::getInstance().resetScheduler(scheduler_name);
  scheduler = SharedScheduler::getInstance().getScheduler();
  scheduler->resize(getNumberOfCoresOnSystem());
}

TEST_P(SchedulerTest, wait_task_test) {
  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();
  scheduler->schedule(waiter);
  waiter->wait();
}

long int getTimeInMillis() {
  /* Linux */
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long int ret = tv.tv_usec;
  /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
  ret /= 1000;
  /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
  ret += (tv.tv_sec * 1000);
  return ret;
}

bool long_block_test(AbstractTaskScheduler * scheduler){
    int threads1 = 2;

    int longSleepTasks = 1;
    int longSleepTime = 100000;
    int shortSleepTasks = 10;
    int shortSleepTime = 10000;
    int waittime = shortSleepTime;

    int upperLimit = longSleepTime / 1000 + (shortSleepTime * shortSleepTasks / (threads1 * 1000)) + waittime/1000;

    scheduler->resize(threads1);

    std::vector<std::shared_ptr<SleepTask> > longTasks;
    std::vector<std::shared_ptr<SleepTask> > shortTasks;
    std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();

    //std::cout << "Number of queues: " << scheduler->getNumberOfWorker() << std::endl;

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

TEST_P(SchedulerTest, dont_block_test) {
  /* we assign a long running task and a number of smaller tasks with a think time to the queues -
     the scheduler should realize that one queue is blocked and assign tasks to other queues / steal work from that queue */

  AbstractTaskScheduler *scheduler;

  scheduler = new SimpleTaskScheduler<CoreBoundTaskQueue>();
  ASSERT_TRUE(long_block_test(scheduler));
  delete scheduler;

  scheduler = new WSSimpleTaskScheduler<WSCoreBoundTaskQueue>();
  ASSERT_TRUE(long_block_test(scheduler));
  delete scheduler;
}

TEST_P(SchedulerTest, sync_task_test) {
  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(2);

  std::shared_ptr<NoOp> nop1 = std::make_shared<NoOp>();
  std::shared_ptr<NoOp> nop2 = std::make_shared<NoOp>();
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
  std::vector<std::shared_ptr<NoOp> > vtasks1;
  std::vector<std::shared_ptr<NoOp> > vtasks2;
  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(threads1);

  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();

  for (int i = 0; i < tasks_group1; ++i) {
    vtasks1.push_back(std::make_shared<NoOp>());
  }
  for (int i = 0; i < tasks_group2; ++i) {
    vtasks2.push_back(std::make_shared<NoOp>());
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
  std::vector<std::shared_ptr<NoOp> > vtasks1;
  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(threads1);

  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();

  for (int i = 0; i < tasks_group1; ++i) {
    vtasks1.push_back(std::make_shared<NoOp>());
    waiter->addDependency(vtasks1[i]);
    scheduler->schedule(vtasks1[i]);
  }

  scheduler->schedule(waiter);
  waiter->wait();
#endif
}

TEST_P(SchedulerTest, wait_dependency_task_test) {
  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(2);
  std::shared_ptr<NoOp> nop = std::make_shared<NoOp>();
  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();
  waiter->addDependency(nop);
  scheduler->schedule(nop);
  scheduler->schedule(waiter);
  waiter->wait();
}

TEST_P(SchedulerTest, resize_simple_test) {
#ifdef EXPENSIVE_TESTS
  int threads1 = 4;
  int threads2 = 2;
  int tasks = 10;
  //in microsecons
  int sleeptime = 50;

  SharedScheduler::getInstance().resetScheduler(scheduler_name);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();
  scheduler->resize(threads1);

  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();
  //std::cout << "Waiter: " << std::hex << (void * )waiter.get() << std::dec << std::endl;
  std::vector<std::shared_ptr<SleepTask> > vtasks;
  for (int i = 0; i < tasks; ++i) {
    vtasks.push_back(std::make_shared<SleepTask>(sleeptime));
    //std::cout << "Task " << i << " :"<< std::hex << (void * )vtasks[i].get() << std::dec << std::endl;
    waiter->addDependency(vtasks[i]);
    scheduler->schedule(vtasks[i]);
  }
  scheduler->schedule(waiter);
  //usleep(sleeptime*1000);
  scheduler->resize(threads2);
  ASSERT_EQ(static_cast<int>(scheduler->getNumberOfWorker()), threads2);
  waiter->wait();

  //std::cout << " Test done " << std::endl;
  //usleep(1000);

#endif
}

TEST_P(SchedulerTest, wait_set_test) {
  //int threads1 = 4;
  int tasks = 100;
  //in microseconds
  int sleeptime = 50;

  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  //scheduler->resize(threads1);

  auto waiter = std::make_shared<WaitTask>();
  auto sleeper = std::make_shared<SleepTask>(sleeptime);

  std::vector<std::shared_ptr<NoOp> > vtasks;
  for (int i = 0; i < tasks; ++i) {
    vtasks.push_back(std::make_shared<NoOp>());
    sleeper->addDependency(vtasks[i]);
    scheduler->schedule(vtasks[i]);
  }
  waiter->addDependency(sleeper);
  scheduler->schedule(sleeper);
  scheduler->schedule(waiter);
  waiter->wait();
}

TEST_P(SchedulerTest, settings_test) {
  int threads1 = getNumberOfCoresOnSystem() - 1;
  int threads2 = getNumberOfCoresOnSystem();

  if(!SharedScheduler::getInstance().isInitialized())
     SharedScheduler::getInstance().init(scheduler_name);
   AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  //std::cout << " Number of queues: " << scheduler->getNumberOfWorker() << std::endl;
  scheduler->resize(threads1);
  //std::cout << " Number of queues: " << scheduler->getNumberOfWorker() << std::endl;
  std::shared_ptr<TaskSchedulerAdjustment> tsa = std::make_shared<TaskSchedulerAdjustment>();
  tsa->setThreadpoolSize(threads2);
  //std::cout << "TSA " << std::hex << (void *)tsa.get() << std::endl;
  std::shared_ptr<WaitTask> waiter = std::make_shared<WaitTask>();
  //std::cout << "Wait " << std::hex << (void *)tsa.get() << std::endl;
  waiter->addDependency(tsa);
  scheduler->schedule(tsa);
  scheduler->schedule(waiter);
  waiter->wait();

  ASSERT_EQ(threads2, static_cast<int>(scheduler->getNumberOfWorker()));

  scheduler->resize(getNumberOfCoresOnSystem());
}

TEST_P(SchedulerTest, singleton_test) {
  int queues1 = getNumberOfCoresOnSystem();
  int queues2 = getNumberOfCoresOnSystem() - 1;

  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler1 = SharedScheduler::getInstance().getScheduler();

  scheduler1->resize(queues1);

  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler2 = SharedScheduler::getInstance().getScheduler();

  ASSERT_EQ(queues1, static_cast<int>(scheduler2->getNumberOfWorker()));
  ASSERT_EQ(scheduler1, scheduler2);

  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler3 = SharedScheduler::getInstance().getScheduler();

  ASSERT_EQ(scheduler1, scheduler3);
  scheduler3->resize(queues2);
  ASSERT_EQ(queues2, static_cast<int>(scheduler2->getNumberOfWorker()));
  scheduler3->resize(getNumberOfCoresOnSystem());
}

TEST_P(SchedulerTest, avoid_too_many_threads_test) {
  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  if (scheduler == NULL) {
    // resize should create not more queues than available cores
    scheduler->resize(getNumberOfCoresOnSystem()+ 1);
    // scheduler should have only as many queues as existing cores
    ASSERT_EQ(getNumberOfCoresOnSystem(), static_cast<int>(scheduler->getNumberOfWorker()));
  }
}

#endif

}


