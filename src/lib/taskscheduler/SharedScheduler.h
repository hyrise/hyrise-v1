// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * SharedScheduler.h
 *
 *  Created on: May 24, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_SHAREDSCHEDULER_H_
#define SRC_LIB_TASKSCHEDULER_SHAREDSCHEDULER_H_

#include <taskscheduler/AbstractTaskScheduler.h>
#include <stdexcept>

struct AbstractTaskSchedulerFactory {
  virtual AbstractTaskScheduler * create(int cores) const = 0;
  virtual ~AbstractTaskSchedulerFactory() {}
};

/// Factory for schedulers, implements abstract factory pattern
template<typename T>
struct TaskSchedulerFactory : public AbstractTaskSchedulerFactory {
  AbstractTaskScheduler * create(int cores) const {
    return new T(cores);
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
  typedef std::map< std::string, AbstractTaskSchedulerFactory * > factory_map_t;
  factory_map_t _schedulers;
  AbstractTaskScheduler * _sharedScheduler;

  SharedScheduler(){
    _sharedScheduler = NULL;
  }

public:

  ~SharedScheduler(){
    _schedulers.clear();
    delete _sharedScheduler;
  }

  template<typename TaskSchedulerClass>
  static bool registerScheduler(const std::string &scheduler){
    auto &sharedScheduler = SharedScheduler::getInstance();
    AbstractTaskSchedulerFactory * factory = new TaskSchedulerFactory<TaskSchedulerClass>();
    sharedScheduler.addScheduler(scheduler, factory);
    return true;
  }

  void addScheduler(const std::string &scheduler, AbstractTaskSchedulerFactory * factory){
    _schedulers[scheduler] = factory;
  }

  bool isInitialized(){
    return (_sharedScheduler != NULL);
  }

  void init(const std::string &scheduler, int cores = getNumberOfCoresOnSystem()){

    if(_sharedScheduler != NULL)
      throw SchedulerException("Scheduler has already been initialized");
    if(_schedulers.find(scheduler) != _schedulers.end()){
      _sharedScheduler = _schedulers[scheduler]->create(cores);
    } else
      throw SchedulerException("Requested scheduler was not registered");
  }

  /*
   * stops current scheduler gracefully; starts new scheduler
   */
  void resetScheduler(const std::string &scheduler, int cores = getNumberOfCoresOnSystem()){
    if(_sharedScheduler != NULL)
      _sharedScheduler->shutdown();
    if(_schedulers.find(scheduler) != _schedulers.end()){
      _sharedScheduler = _schedulers[scheduler]->create(cores);
    } else
      throw SchedulerException("Requested scheduler was not registered");
  }

  AbstractTaskScheduler * getScheduler() {
    return _sharedScheduler;
  }

  static SharedScheduler &getInstance();
};

#endif  // SRC_LIB_TASKSCHEDULER_SHAREDSCHEDULER_H_
