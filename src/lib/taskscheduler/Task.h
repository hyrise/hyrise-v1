// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * Task.h
 *
 *  Created on: Feb 14, 2012
 *      Author: jwust
 */

#pragma once

#include <vector>
#include <memory>
#include <condition_variable>
#include <string>

#include "helper/locking.h"
#include "helper/types.h"

namespace hyrise {
namespace taskscheduler {

class Task;

class TaskReadyObserver {
  /*
   * notify that task has changed state
   */
public:
  virtual void notifyReady(task_ptr_t task) = 0;
  virtual ~TaskReadyObserver() {
  };
};

class TaskDoneObserver {
  /*
   * notify that task has changed state
   */
public:
  virtual void notifyDone(task_ptr_t task) = 0;
  virtual ~TaskDoneObserver() {
  };
};

/*
 * a task that can be scheduled by a Task Scheduler
 */
class Task : public TaskDoneObserver, public std::enable_shared_from_this<Task> {

public:
  static const int DEFAULT_PRIORITY = 999;
  static const int HIGH_PRIORITY = 1;
  static const int NO_PREFERRED_CORE = -1;
  static const int NO_PREFERRED_NODE = -1;
  static const int SESSION_ID_NOT_SET = 0;
  // split up the operator in as many instances as indicated by dynamicCount
  virtual std::vector<task_ptr_t> applyDynamicParallelization(size_t dynamicCount);
  // determine the number of instances necessary to adhere to a max task size.
  // should be overridden in operators
  // currently all implementing operators are fine-tuned for server gaza.
  virtual size_t determineDynamicCount(size_t maxTaskRunTime) {
    return 1;
  }

protected:
  std::vector<task_ptr_t> _dependencies;
  std::vector<std::weak_ptr<TaskReadyObserver>> _readyObservers;
  std::vector<std::weak_ptr<TaskDoneObserver>> _doneObservers;

  int _dependencyWaitCount;
  // mutex for dependencyCount and dependency vector
  hyrise::locking::Spinlock _depMutex;
  // mutex for observer vector
  hyrise::locking::Spinlock _observerMutex;
  // mutex to stop notifications, while task is being scheduled to wait set in SimpleTaskScheduler
  hyrise::locking::Spinlock _notifyMutex;
  // indicates on which core the task should run
  int _preferredCore;
  // indicates on which node the task should run
  int _preferredNode;
  // indicates on which node the task should run
  int _actualNode;
  // priority
  int _priority;
  // sessionId
  int _sessionId;
  // id - equals transaction id
  int _id;

  // if true, the DynamicPriorityScheduler will determine the number of instances.
  bool _dynamic = false;

public:
  Task();
  virtual ~Task() {};
  virtual void operator()() {};

  /*
   * currently, only Response Task overrides this function;
   */
  virtual double getQueryDuration() const {return 0;}

  /*
    Workaround that allows to retrieve the name of a task during runtime, since there is no way to call the
    static member function name() that we use for the plan op factory.
    name is assumed to be unique
   */
  virtual const std::string vname() = 0;
  /*
   * if tasks rae waiting for this task to finish
   */
  bool hasSuccessors();
  /*
   * adds dependency; the task is ready to run if all tasks this tasks depends on are finished
   */
  void addDependency(task_ptr_t dependency);
  /*
   * adds dependency, but do not increase dependencyWaitCount or register as DoneObserver, as dependency is known to be done
   */
  void addDoneDependency(task_ptr_t dependency);
  /*
   * removes dependency; 
   */
  void removeDependency(task_ptr_t dependency);
  /*
   * change dependency; 
   */
  void changeDependency(task_ptr_t from, task_ptr_t to);

  /*
   * gets the number of dependencies
   */
  int getDependencyCount();
  /*
   * set dependencies directly and not managed by Task; make sure dependency count and dependencies match;
   * currently used to set dependencies for a task that is ready to run (no unmet dependencies), but needs to get inputs
   */
   void setDependencies(std::vector<task_ptr_t> dependencies, int count);
  /*
   * check if the supplied task is a direct dependency of this task.
   */
  bool isDependency(const task_ptr_t& task);
  /*
   * adds an observer that gets notified if this task is ready to run
   */
  void addReadyObserver(const std::shared_ptr<TaskReadyObserver>& observer);
  /*
   * adds an obserer that gets notified if this task is done
   */
  void addDoneObserver(const std::shared_ptr<TaskDoneObserver>& observer);
  /*
   * whether this task is ready to run / has open dependencies
   */
  bool isReady();
  /*
   * notify that task is done
   */
  void notifyDone(task_ptr_t task);
  /*
   * notify all ready observers that task is ready
   */
  void notifyReadyObservers();
  /*
   * notify all done observers that task is done
   */
  void notifyDoneObservers();
  /*
   * specify preferred core for task
   */
  void setPreferredCore(int core);
  /*
   * get preferred core for this task
   */
  int getPreferredCore();
  /*
   * block task for notifications -> used e.g., when task is moved into wait set of scheduler
   */
  void lockForNotifications();
  void unlockForNotifications();

  int getActualNode() const {
    return _actualNode;
  }

  void setActualNode(int actualNode) {
    _actualNode = actualNode;
  }

  int getPreferredNode() const {
    return _preferredNode;
  }

  void setPreferredNode(int preferredNode) {
    _preferredNode = preferredNode;
  }

  int getPriority() const {
    return _priority;
  }

  void setPriority(int priority) {
    this->_priority = priority;
  }

  int getId() const {
    return _id;
  }

  void setId(int id) {
    this->_id = id;

  }
  int getSessionId() const
  {
    return _sessionId;
  }

  void setSessionId(int sessionId)
  {
    _sessionId = sessionId;
  }

  // used in the DynamicPriorityScheduler
  // if true and task is ParallizablePlanOperation the number of instances is determined
  // by an operators determineDynamicCount operation.
  void setDynamic(bool dynamic) {_dynamic = dynamic;}
  bool isDynamic() {return _dynamic;}
};

class CompareTaskPtr {
    public:
    bool operator()(const task_ptr_t & t1, const task_ptr_t & t2) // Returns true if t1 is lower priority than t2
    {
       if (t1->getPriority() > t2->getPriority()) return true;
       else if((t1->getPriority() == t2->getPriority()) && t1->getId() > t2->getId()) return true;
       else
         return false;
    };
};

class WaitTask : public Task {
private:
  bool _finished;
  hyrise::locking::Spinlock _mut;
  std::condition_variable_any _cond;
public:
  WaitTask();
  virtual ~WaitTask() {};

  virtual void operator()();
  void wait();
  const std::string vname(){return "WaitTask";};
};

class SleepTask : public Task {
private:
  int _microseconds;
public:
  explicit SleepTask(int microseconds);
  virtual ~SleepTask() {};
  virtual void operator()();
  const std::string vname(){return "SleepTask";};
};

class SyncTask : public Task {
public:
  SyncTask() {};
  virtual ~SyncTask() {};
  virtual void operator()();

  const std::string vname(){return "SyncTask";};
};

} } // namespace hyrise::taskscheduler

