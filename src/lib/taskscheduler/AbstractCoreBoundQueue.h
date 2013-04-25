/*
 * AbstractCoreBoundQueue.h
 *
 *  Created on: Apr 4, 2013
 *      Author: jwust
 */

#ifndef ABSTRACTCOREBOUNDQUEUE_H_
#define ABSTRACTCOREBOUNDQUEUE_H_

#include "taskscheduler/AbstractTaskQueue.h"
#include <atomic>

class AbstractCoreBoundQueue : public AbstractTaskQueue{

protected:
  // worker thread
  std::thread *_thread;
  // the core the thread should run on
  std::atomic<queue_status_t> _status;
  // specific core thread is bound to
  int _core;
  // mutex to protect the queue
  std::mutex _queueMutex;
  // mutext to protect the thread status
  std::mutex _threadStatusMutex;
  // condition variable to wake up thread
  std::condition_variable _condition;

  static log4cxx::LoggerPtr logger;
  /*
   * launch a new thread
   */
  void launchThread(int core);

  AbstractCoreBoundQueue();

public:

  virtual ~AbstractCoreBoundQueue();

  /**
   * empty queue
   */
  virtual std::vector<std::shared_ptr<Task> > emptyQueue() = 0;

  /*
   * wait until all tasks are done
   */
  void join();

  /*
   * stops queue w/o waiting until all tasks are done
   */
  virtual std::vector<std::shared_ptr<Task> > stopQueue() = 0;

  // getter/setter
  int getCore() const{
    return _core;
  }
};

#endif /* ABSTRACTCOREBOUNDQUEUE_H_ */

