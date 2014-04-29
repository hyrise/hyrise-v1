// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "testing/test.h"
#include "access/PipelineObserver.h"

#include "pipelining.h"

#include "taskscheduler/ThreadPerTaskScheduler.h"
#include "taskscheduler/Task.h"

#include "helper/types.h"
#include "helper/Synchronized.h"

namespace hyrise {
namespace access {

/* Manual mock object.
 * Uses standard copy and execute implementation of PipelineObserver,
 * and also stores all created copies.
 */
class SpyObserver : public PlanOperation, public PipelineObserver<SpyObserver> {

 public:
  virtual planop_ptr_t copy() override {
    auto copy = std::make_shared<SpyObserver>();
    _copies([&copy](std::set<planop_ptr_t>& copies) { copies.insert(copy); });
    return copy;
  };

  virtual void executePlanOperation() override{};

  const std::set<planop_ptr_t> getCopies() {
    return _copies([](std::set<planop_ptr_t> & copies)
                       ->std::set<planop_ptr_t> { return std::set<planop_ptr_t>(copies); });
  };

 private:
  Synchronized<std::set<planop_ptr_t>, std::mutex> _copies;
};

TEST(PipelineObserverTest, spawnCopyOnReceive) {
  auto emitter = std::make_shared<TestEmitter>();
  auto observer = std::make_shared<SpyObserver>();

  observer->addDependency(emitter);

  auto waiter = std::make_shared<taskscheduler::WaitTask>();
  waiter->addDependency(observer);

  taskscheduler::SharedScheduler::getInstance().resetScheduler("ThreadPerTaskScheduler");
  const auto& scheduler = taskscheduler::SharedScheduler::getInstance().getScheduler();
  std::vector<taskscheduler::task_ptr_t> query{emitter, observer, waiter};
  scheduler->scheduleQuery(query);

  waiter->wait();

  auto observerCopies = observer->getCopies();

  std::set<storage::c_atable_ptr_t> uniqueChunks;
  for (const auto& copy : observerCopies) {
    uniqueChunks.insert(copy->getInputTable());
  }

  EXPECT_EQ(emitter->getNumChunks(), uniqueChunks.size());
}
}
}
