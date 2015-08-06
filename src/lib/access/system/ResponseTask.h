// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_RESPONSETASK_H_
#define SRC_LIB_ACCESS_RESPONSETASK_H_

#include <atomic>
#include <mutex>

#include "json.h"

#include "helper/epoch.h"
#include "access/system/OutputTask.h"
#include "net/AbstractConnection.h"
#include "io/TXContext.h"
#include "access/ScriptOperation.h"

namespace hyrise {
namespace access {

class PlanOperation;

class ResponseTask : public taskscheduler::Task {
 private:
  net::AbstractConnection* connection;

  size_t _transmitLimit = 0;  // Used for serialization only
  size_t _transmitOffset = 0;  // Used for serialization only

  // Indicates which dependency contains the result
  int _resultTaskIndex = 0;

  std::atomic<unsigned long> _affectedRows;
  tx::TXContext _txContext;
  epoch_t queryStart = 0;
  bool _isAutoCommit = false;
  performance_vector_t performance_data;

  // Unique refs to the generated keys of all planops
  std::vector<std::unique_ptr<std::vector<hyrise_int_t>>> _generatedKeyRefs;

  std::mutex perfMutex;
  std::mutex errorMutex;
  std::vector<std::string> _error_messages;

  bool _recordPerformanceData = true;

  bool _group_commit = false;

  bool _getSubQueryPerformanceData;

  std::shared_ptr<ScriptOperation> _scriptOperation;

 public:
  explicit ResponseTask(net::AbstractConnection* connection)
      : connection(connection), _getSubQueryPerformanceData(false) {
    _affectedRows = 0;
  }

  virtual ~ResponseTask() {}

  const std::string vname();

  void setRecordPerformanceData(bool val) { _recordPerformanceData = val; }

  epoch_t getQueryStart() { return queryStart; }

  void registerPlanOperation(const std::shared_ptr<PlanOperation>& planOp);

  void addErrorMessage(std::string message) {
    std::lock_guard<std::mutex> guard(errorMutex);
    _error_messages.push_back(message);
  }

  std::vector<std::string> getErrorMessages() const { return _error_messages; }
  void setIsAutoCommit(bool b) { _isAutoCommit = b; }

  void setTxContext(tx::TXContext t) { _txContext = t; }

  tx::TXContext getTxContext() const { return _txContext; }

  void setQueryStart(epoch_t start) { queryStart = start; }

  void setTransmitLimit(size_t l) { _transmitLimit = l; }

  void setTransmitOffset(size_t o) { _transmitOffset = o; }

  int getResultTaskIndex() { return _resultTaskIndex; }

  void setResultTaskIndex(int i) { _resultTaskIndex = i; }

  void incAffectedRows(unsigned long inc) { _affectedRows += inc; }

  performance_vector_t& getPerformanceData() { return performance_data; }

  task_states_t getState() const;

  std::shared_ptr<PlanOperation> getResultTask();

  void setGroupCommit(bool group_commit);

  void enableGetSubQueryPerformanceData(std::shared_ptr<ScriptOperation> scriptOperation) {
    _getSubQueryPerformanceData = true;
    _scriptOperation = scriptOperation;
  }

  Json::Value generateResponseJson();

  virtual void operator()();
};
}
}

#endif  // SRC_LIB_ACCESS_RESPONSETASK_H_
