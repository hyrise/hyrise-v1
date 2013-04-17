// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_OUTPUTTASK_H_
#define SRC_LIB_ACCESS_OUTPUTTASK_H_

#include <log4cxx/logger.h>

#include <stdint.h>
#include <time.h>
#include <vector>
#include <map>
#include <utility>
#include <json.h>

#include "helper/epoch.h"
#include "taskscheduler/Task.h"

// A execution task can end up in different states, if the state is
// set to OpFail, the result of the operation will be discarded and
// the error message will be forwared until the plan is finished, all
// subsequent plan operations will not be executed.
typedef enum {
  OpFail = 0,
  OpUnknown = 1,
  OpSuccess = 2
} task_states_t;

class OutputTask : public Task {

 public:

  // Struct used for performance data
  typedef struct {
    int64_t duration;
    int64_t data;
    std::string papiEvent;
    std::string name;
    std::string operatorId;
    epoch_t startTime;
    epoch_t endTime;
    std::string executingThread;

  } performance_attributes_t;

  typedef std::vector<std::unique_ptr<performance_attributes_t> > performance_vector;

 protected:
  performance_attributes_t *_performance_attr;

  // Indicate whether an operation has failed
  // Subclass has to explicitly set on success
  task_states_t _state;

  std::string _error_message;

  std::string _papiEvent;
 public:

  OutputTask() : _performance_attr(nullptr), _state(OpUnknown), _papiEvent("PAPI_TOT_INS") {
  };

  virtual ~OutputTask() {
  }

  task_states_t getState() const {
    return _state;
  }

  void setState(task_states_t state) {
    _state = state;
  }

  std::string getErrorMessage() const {
    return _error_message;
  }

  void setErrorMessage(std::string error_message) {
    _error_message = error_message;
  }

  performance_attributes_t &getPerformanceData() {
    return *_performance_attr;
  };
  void setPerformanceData(performance_attributes_t *attr) {
    _performance_attr = attr;
  }

  void setEvent(std::string ev) {
    _papiEvent = ev;
  }

  const std::string getEvent() const {
    return _papiEvent;
  }

  virtual void operator()() { }
};

#endif  // SRC_LIB_ACCESS_OUTPUTTASK_H_

