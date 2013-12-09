// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_OUTPUTTASK_H_
#define SRC_LIB_ACCESS_OUTPUTTASK_H_

#include <stdint.h>
#include <time.h>
#include <vector>
#include <map>
#include <utility>

#include <storage/storage_types.h>
#include "helper/epoch.h"
#include "taskscheduler/Task.h"

namespace hyrise { namespace access {

// A execution task can end up in different states, if the state is
// set to OpFail, the result of the operation will be discarded and
// the error message will be forwared until the plan is finished, all
// subsequent plan operations will not be executed.
typedef enum {
  OpFail = 0,
  OpUnknown = 1,
  OpSuccess = 2
} task_states_t;


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

typedef std::vector<std::unique_ptr<performance_attributes_t>> performance_vector_t;

class OutputTask : public taskscheduler::Task {
 protected:

  performance_attributes_t *_performance_attr = nullptr;
  // Indicate whether an operation has failed
  // Subclass has to explicitly set on success
  task_states_t _state = OpUnknown;
  
  std::string _papiEvent = "PAPI_TOT_INS";

  // Collect the generated keys
  std::vector<hyrise_int_t> *_generatedKeys;

 public:
  task_states_t getState() const {
    return _state;
  }

  void setState(task_states_t state) {
    _state = state;
  }

  performance_attributes_t& getPerformanceData() {
    return *_performance_attr;
  }
  
  void setPerformanceData(performance_attributes_t *attr) {
    _performance_attr = attr;
  }

  void setGeneratedKeysData(std::vector<hyrise_int_t>* d) {
    _generatedKeys = d;
  }

  void setEvent(std::string ev) {
    _papiEvent = ev;
  }

  const std::string getEvent() const {
    return _papiEvent;
  }
};

}}
#endif  // SRC_LIB_ACCESS_OUTPUTTASK_H_

