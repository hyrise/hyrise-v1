// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef DYNAMICPRIORITYSCHEDULER_H_
#define DYNAMICPRIORITYSCHEDULER_H_

#include "CentralPriorityScheduler.h"

namespace hyrise {
namespace taskscheduler {

class DynamicPriorityScheduler : public CentralPriorityScheduler {

public:
  DynamicPriorityScheduler (int threads = getNumberOfCoresOnSystem());

  virtual void notifyReady(std::shared_ptr<Task> task);

  virtual void schedule(std::shared_ptr<Task> task);
  
  void setMaxTaskSize(size_t maxTaskSize) {
    _maxTaskSize = maxTaskSize;
  }

private:
  size_t _maxTaskSize = 0;
};

}}

#endif /* DYNAMICPRIORITYSCHEDULER_H_ */
