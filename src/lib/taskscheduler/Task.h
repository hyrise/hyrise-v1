// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * Task.h
 *
 *  Created on: Feb 14, 2012
 *      Author: jwust
 */

#ifndef SRC_LIB_TASKSCHEDULER_TASK_H_
#define SRC_LIB_TASKSCHEDULER_TASK_H_

#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <string>

#define NO_PREFERRED_CORE -1
#define NO_PREFERRED_NODE -1

class Task;

class TaskReadyObserver {
  /*
   * notify that task has changed state
   */
public:
  virtual void notifyReady(std::shared_ptr<Task> task) = 0;
  virtual ~TaskReadyObserver() {
  };
};

class TaskDoneObserver {
  /*
   * notify that task has changed state
   */
public:
  virtual void notifyDone(std::shared_ptr<Task> task) = 0;
  virtual ~TaskDoneObserver() {
  };
};

/*
 * a task that can be scheduled by a Task Scheduler
 */
class Task : public TaskDoneObserver, public std::enable_shared_from_this<Task> {

public:

protected:
  std::vector<std::shared_ptr<Task> > _dependencies;
  std::vector<TaskReadyObserver *> _readyObservers;
  std::vector<TaskDoneObserver *> _doneObservers;

  int _dependencyWaitCount;
  // mutex for dependencyCount and dependency vector
  std::mutex _depMutex;
  // mutex for observer vector
  std::mutex _observerMutex;
  // mutex to stop notifications, while task is being scheduled to wait set in SimpleTaskScheduler
  std::mutex _notifyMutex;
  // indicates on which core the task should run
  int _preferredCore;
  // indicates on which node the task should run
  int _preferredNode;
  // indicates on which node the task should run
  int _actualNode;

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
  void addDependency(std::shared_ptr<Task> dependency);
  /*
   * gets the number of dependencies
   */
  int getDependencyCount();
  /*
   * adds an observer that gets notified if this task is ready to run
   */
  void addReadyObserver(TaskReadyObserver *observer);
  /*
   * adds an obserer that gets notified if this task is done
   */
  void addDoneObserver(TaskDoneObserver *observer);
  /*
   * whether this task is ready to run / has open dependencies
   */
  bool isReady();
  /*
   * notify that task is done
   */
  void notifyDone(std::shared_ptr<Task> task);
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
};

class WaitTask : public Task {
private:
  bool _finished;
  std::mutex _mut;
  std::condition_variable _cond;
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

#endif  // SRC_LIB_TASKSCHEDULER_TASK_H_
