// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_RESPONSETASK_H_
#define SRC_LIB_ACCESS_RESPONSETASK_H_

#include <mutex>

#include "helper/epoch.h"
#include "access/system/OutputTask.h"
#include "net/AbstractConnection.h"

namespace hyrise {
namespace access {

class PlanOperation;

class ResponseTask : public Task {
 private:
  net::AbstractConnection *connection;
  size_t _transmitLimit; // Used for serialization only
  epoch_t queryStart;
  performance_vector_t performance_data;
  std::mutex perfMutex;
  std::mutex errorMutex;
  std::vector<std::string> _error_messages;
 public:
  explicit ResponseTask(net::AbstractConnection *connection) :
      connection(connection), _transmitLimit(0), queryStart(0) {
  }

  virtual ~ResponseTask() {}

  const std::string vname();

  epoch_t getQueryStart() {
    return queryStart;
  }

  void registerPlanOperation(const std::shared_ptr<PlanOperation>& planOp);

  void addErrorMessage(std::string message) {
    std::lock_guard<std::mutex> guard(errorMutex);
    _error_messages.push_back(message);
  }

  std::vector<std::string> getErrorMessages() const {
    return _error_messages;
  }
  
  void setQueryStart(epoch_t start) {
    queryStart = start;
  }

  void setTransmitLimit(size_t l) {
    _transmitLimit = l;
  }

  performance_vector_t& getPerformanceData() {
    return performance_data;
  }

  task_states_t getState() const;
  
  std::shared_ptr<PlanOperation> getResultTask();
  
  virtual void operator()();
};

}
}

#endif  // SRC_LIB_ACCESS_RESPONSETASK_H_
