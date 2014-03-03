// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "testing/test.h"
#include "access/PipelineObserver.h"

#include "taskscheduler/ThreadPerTaskScheduler.h"

#include "helper/Synchronized.h"

#include "pipelining.h"

#include <iostream>

namespace hyrise {
namespace access {

/*
 * Manual mock object.
 * Will store all unique received chunks and provides a count of them.
 */
class TestObserver : public PlanOperation, public PipelineObserver<TestObserver> {
 public:
  void executePlanOperation() override {}
  void notifyNewChunk(storage::c_aresource_ptr_t chunkTbl) override {
    _receivedChunks([&chunkTbl](std::set<storage::c_aresource_ptr_t>& receivedChunks) {
      receivedChunks.insert(chunkTbl);
    });
  }

  size_t numReceivedChunks() {
    return _receivedChunks([](std::set<storage::c_aresource_ptr_t> & receivedChunks)
                               ->size_t { return receivedChunks.size(); });
  }

 private:
  Synchronized<std::set<storage::c_aresource_ptr_t>, std::mutex> _receivedChunks;
};

TEST(PipelineEmitterTest, notifiesAllObserver) {
  auto emitter = std::make_shared<TestEmitter>();
  auto obs1 = std::make_shared<TestObserver>();
  auto obs2 = std::make_shared<TestObserver>();

  obs1->addDependency(emitter);
  obs2->addDependency(emitter);

  auto waiter = std::make_shared<taskscheduler::WaitTask>();
  waiter->addDependency(obs1);
  waiter->addDependency(obs2);

  const auto& scheduler = std::make_shared<taskscheduler::ThreadPerTaskScheduler>();
  scheduler->scheduleQuery({emitter, obs1, obs2, waiter});

  waiter->wait();

  EXPECT_EQ(emitter->getNumChunks(), obs1->numReceivedChunks());
  EXPECT_EQ(emitter->getNumChunks(), obs2->numReceivedChunks());
};
}
}
