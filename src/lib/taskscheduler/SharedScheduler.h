// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * SharedScheduler.h
 *
 *  Created on: May 24, 2012
 *      Author: jwust
 */

#pragma once

#include <taskscheduler/AbstractTaskScheduler.h>
#include <taskscheduler/DynamicPriorityScheduler.h>
#include <stdexcept>

namespace hyrise {
namespace taskscheduler {

struct AbstractTaskSchedulerFactory {
  virtual std::shared_ptr<AbstractTaskScheduler> create(int cores) const = 0;
  virtual ~AbstractTaskSchedulerFactory() {}
};

/// Factory for schedulers, implements abstract factory pattern
template<typename T>
struct TaskSchedulerFactory : public AbstractTaskSchedulerFactory {
  std::shared_ptr<AbstractTaskScheduler> create(int cores) const {
    return std::shared_ptr<AbstractTaskScheduler>(new T(cores));
  }
};

/// For all Scheduler related exceptions
class SchedulerException : public std::runtime_error {
public:
  explicit SchedulerException(const std::string &message): std::runtime_error(message){}
};

/*
 * Singleton; provides reference to a shared scheduler object; scheduler is set by string; schedulers can registers
 */
class SharedScheduler{
  typedef std::map< std::string, std::unique_ptr<AbstractTaskSchedulerFactory>> factory_map_t;
  factory_map_t _schedulers;
  std::shared_ptr<AbstractTaskScheduler> _sharedScheduler;

public:

  ~SharedScheduler(){
    _schedulers.clear();
  }

  template<typename TaskSchedulerClass>
  static bool registerScheduler(const std::string &scheduler){
    auto &sharedScheduler = SharedScheduler::getInstance();
    AbstractTaskSchedulerFactory * factory = new TaskSchedulerFactory<TaskSchedulerClass>();
    sharedScheduler.addScheduler(scheduler, factory);
    return true;
  }

  void addScheduler(const std::string &scheduler, AbstractTaskSchedulerFactory * factory){
    _schedulers[scheduler].reset(factory);
  }

  bool isInitialized(){
    return bool(_sharedScheduler);
  }

  void init(const std::string &scheduler, int cores = getNumberOfCoresOnSystem(), int maxTaskSize = 0){

    if(_sharedScheduler)
      throw SchedulerException("Scheduler has already been initialized");
    if(_schedulers.find(scheduler) != _schedulers.end()){
      _sharedScheduler = _schedulers[scheduler]->create(cores);
      if (auto dynamicScheduler = std::dynamic_pointer_cast<DynamicPriorityScheduler>(_sharedScheduler)) {
      	dynamicScheduler->setMaxTaskSize(maxTaskSize);
      }
    } else
      throw SchedulerException("Requested scheduler was not registered");
  }

  /*
   * stops current scheduler gracefully; starts new scheduler
   */
  void resetScheduler(const std::string &scheduler, int cores = getNumberOfCoresOnSystem()){
    if(_sharedScheduler) {
      _sharedScheduler->shutdown();
    }
    
    if(_schedulers.find(scheduler) != _schedulers.end()){
      _sharedScheduler = _schedulers[scheduler]->create(cores);
    } else
      throw SchedulerException("Requested scheduler was not registered");
  }

  std::shared_ptr<AbstractTaskScheduler> getScheduler() {
    return _sharedScheduler;
  }

  static SharedScheduler &getInstance();
};

} } // namespace hyrise::taskscheduler

