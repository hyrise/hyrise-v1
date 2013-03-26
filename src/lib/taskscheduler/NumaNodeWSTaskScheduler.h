#ifndef SRC_LIB_TASKSCHEDULER_NUMANODEWSTASKSCHEDULER_H_
#define SRC_LIB_TASKSCHEDULER_NUMANODEWSTASKSCHEDULER_H_

#include <taskscheduler/AbstractTaskScheduler.h>
#include <taskscheduler/NumaNodeWSCoreBoundTaskQueue.h>
#include <deque>
#include <iostream>
#include "helper/HwlocHelper.h"

template<class TaskQueue>
class NumaNodeWSTaskScheduler : public AbstractQueueBasedTaskScheduler<TaskQueue> {
protected:

  unsigned _numaNodes;
  std::map<unsigned, std::vector<unsigned> > _numaQueues;
  static bool registered;
  unsigned _nextNumaNode;
  std::vector<unsigned> _nextNumaQueue;
  unsigned _coresPerNumaNode;

public:
  typedef typename AbstractQueueBasedTaskScheduler<TaskQueue>::task_queues_t task_queues_t;
  typedef typename AbstractQueueBasedTaskScheduler<TaskQueue>::task_queue_t task_queue_t;
  typedef typename AbstractQueueBasedTaskScheduler<TaskQueue>::scheduler_status_t scheduler_status_t;

  NumaNodeWSTaskScheduler(const int queues = getNumberOfCoresOnSystem()): AbstractQueueBasedTaskScheduler<TaskQueue>() {
    _numaNodes = getNumberOfNodes(getHWTopology());
    _nextNumaNode = 0;
    _coresPerNumaNode = getNumberOfCoresPerNumaNode();
    for(unsigned i = 0; i < _numaNodes; i++){
      _numaQueues.insert( std::pair<unsigned,std::vector<unsigned> >(i,getCoresForNode(getHWTopology(),i)));
      _nextNumaQueue.push_back(0);
    }
    // call resizeQueues here and not in AbstractQueueBasedTaskScheduler, as resizeQueue calls virtual createTaskQueue
    this->resize(queues);
  }

  virtual ~NumaNodeWSTaskScheduler() {
    this->_statusMutex.lock();
    this->_status = AbstractQueueBasedTaskScheduler<TaskQueue>::TO_STOP;
    this->_statusMutex.unlock();
    task_queue_t *queue;
    for (size_t i = 0; i < this->_queues; ++i) {
      queue =   this->_taskQueues[i];
      queue->stopQueue();
      delete queue;
    }
  }

  const task_queues_t getTaskQueuesInSameNumaNode(unsigned node) {
    task_queues_t queues;
    // check if task scheduler is about to change structure of scheduler (change number of queues); if yes, return NULL
    {
      std::lock_guard<std::mutex> lk2(this->_statusMutex);
      if (this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::RESIZING
          || this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::TO_STOP
          || this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::STOPPED)
        return queues;
    }
    // return const reference to task queues
    std::lock_guard<std::mutex> lk(this->_queuesMutex);
    auto it = _numaQueues.find(node);
    if( it != _numaQueues.end())
      for(size_t s = 0, c = it->second.size(); s < c; s++){
        queues.push_back(this->_taskQueues.at(it->second.at(s)));

      }
    return queues;
  }

  const task_queues_t getTaskQueuesInOtherNumaNodes(unsigned node) {
    task_queues_t queues;
    // check if task scheduler is about to change structure of scheduler (change number of queues); if yes, return NULL
    {
      std::lock_guard<std::mutex> lk2(this->_statusMutex);
      if (this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::RESIZING
          || this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::TO_STOP
          || this->_status == AbstractQueueBasedTaskScheduler<TaskQueue>::STOPPED)
        return queues;
    }
    // return const reference to task queues
    std::lock_guard<std::mutex> lk(this->_queuesMutex);
    for (auto it=_numaQueues.begin(); it!=_numaQueues.end(); ++it){
      if(it->first != node)
        for(size_t s = 0, c = it->second.size(); s < c; s++){
          queues.push_back(this->_taskQueues.at(it->second.at(s)));
        }
    }
    return queues;
  }

  unsigned getNumaNodes() const {
    return _numaNodes;
  }

  unsigned incQueueInNumeNode(unsigned node){
    //std::cout << " incQueueInNumeNode  PRE: _nextNumaQueue["<<node<<"]=" << _nextNumaQueue[node] << std::endl;
    _nextNumaQueue[node] = (_nextNumaQueue[node] + 1) % _coresPerNumaNode;
    //std::cout << " incQueueInNumeNode  POST _nextNumaQueue["<<node<<"]=" << _nextNumaQueue[node] << std::endl;
    return _nextNumaQueue[node];
  }

  unsigned incNumaNode(){
    _nextNumaNode = (_nextNumaNode + 1) % _numaNodes;
    return _nextNumaNode;
  }

  unsigned nextQueueForNode(unsigned node){
    //std::cout << "nextQueueForNode:   node= " << node << " ;_nextNumaQueue[node]=" << _nextNumaQueue[node] << std::endl;
    unsigned next = 0;
    auto it = _numaQueues.find(node);
    next = (it->second).at(_nextNumaQueue[node]);
    //std::cout << "next= " << next << std::endl;
    return next;
  }


protected:
  void pushToQueue(std::shared_ptr<Task> task) {
    int core = task->getPreferredCore();
    int node = task->getPreferredNode();
    int nextCore;
    std::lock_guard<std::mutex> lk2(this->_queuesMutex);
    //std::cout << "PUSH: node= " << node << " ;core=" << core << std::endl;

    // case 1: no core, no node provided
    if(core == NO_PREFERRED_CORE && node == NO_PREFERRED_NODE){
      //std::cout << "case 1" << std::endl;
      // push task to next node and next core
      nextCore = nextQueueForNode(_nextNumaNode);
      this->_taskQueues[nextCore]->push(task);
      incQueueInNumeNode(_nextNumaNode);
      incNumaNode();
    }
    // case 2: node, no core provided
    else if(core == NO_PREFERRED_CORE && node != NO_PREFERRED_NODE){
      //std::cout << "case 2" << std::endl;
      // push task to preferred node and next core
      nextCore = nextQueueForNode(node);
      this->_taskQueues[nextCore]->push(task);
      incQueueInNumeNode(node);
    }
    // case 3: core, no node provided
    // case 4: core and node provided (ignore node)
    else {
      //std::cout << "case 3" << std::endl;

      // push task to preferred core
      this->_taskQueues[core]->push(task);
    }
  }

  void stopQueueAndRedistributeTasks(task_queue_t *queue, int queues) {
    std::deque<std::shared_ptr<Task> > tmp = queue->stopQueue();
    //redistribute tasks to other queues
    if (tmp.size() > 0) {
      int tmp_size = tmp.size();
      {
        for (int i = 0; i < tmp_size; ++i) {
          // set preferred core to "NO_PREFERRED_CORE"
          // set preferred core to "NO_PREFERRED_CORE" (as queue with preferred core does not exist anymore / is used for other class of tasks)
          tmp.front()->setPreferredCore(NO_PREFERRED_CORE);
          pushToQueue(tmp.front());
          tmp.pop_front();
        }
      }
    }
  }

  virtual task_queue_t *createTaskQueue(int core) {
    return new task_queue_t(core, this);
  }



};

#endif  // SRC_LIB_TASKSCHEDULER_NUMANODEWSTASKSCHEDULER_H_
