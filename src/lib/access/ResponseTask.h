// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_RESPONSETASK_H_
#define SRC_LIB_ACCESS_RESPONSETASK_H_

#include <mutex>

#include "helper/epoch.h"
#include "access/OutputTask.h"
#include "net/AbstractConnection.h"

class _PlanOperation;

namespace hyrise {
namespace access {

class ResponseTask : public Task {
 private:
  net::AbstractConnection *connection;
  size_t _transmitLimit; // Used for serialization only
  epoch_t queryStart;
  OutputTask::performance_vector performance_data;
  std::mutex perfMutex;

 public:
  explicit ResponseTask(net::AbstractConnection *connection) :
      connection(connection), _transmitLimit(0), queryStart(0) {
  }

  virtual ~ResponseTask() {}

  const std::string vname();

  epoch_t getQueryStart() {
    return queryStart;
  }

  void registerPlanOperation(const std::shared_ptr<_PlanOperation>& planOp);

  void setQueryStart(epoch_t start) {
    queryStart = start;
  }

  void setTransmitLimit(size_t l) {
    _transmitLimit = l;
  }

  OutputTask::performance_vector& getPerformanceData() {
    return performance_data;
  }

  std::shared_ptr<_PlanOperation> getResultTask();
  
  virtual void operator()();
};

}
}

#endif  // SRC_LIB_ACCESS_RESPONSETASK_H_
