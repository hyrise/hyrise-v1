// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/ResponseTask.h"

#include <thread>

#include "json.h"
#include "log4cxx/logger.h"
#include "boost/lexical_cast.hpp"

#include "access/system/PlanOperation.h"
#include "access/system/OutputTask.h"
#include "io/TransactionManager.h"
#include "helper/PapiTracer.h"

#include "net/AsyncConnection.h"

#include "storage/AbstractTable.h"
#include "storage/SimpleStore.h"
#include "storage/meta_storage.h"


namespace hyrise {
namespace access {

namespace {
log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.net"));
}

template <typename T>
struct json_functor {
  typedef Json::Value value_type;

  const T& table;
  size_t column;
  size_t row;

  explicit json_functor(const T& t): table(t), column(0), row(0) {}

  template <typename R>
  value_type operator()() {
    return Json::Value(table->template getValue<R>(column, row));
  }
};

template<typename T>
Json::Value generateRowsJsonT(const T& table, const size_t transmitLimit, const size_t transmitOffset) {
  storage::type_switch<hyrise_basic_types> ts;
  json_functor<T> fun(table);
  Json::Value rows(Json::arrayValue);
  for (size_t row = 0; row < table->size(); ++row) {

    // Align offset
    if (row < transmitOffset)
      continue;

    // Break if limit reached
    if (transmitLimit > 0 && row == (transmitOffset + transmitLimit))
      break;

    fun.row = row;
    Json::Value json_row(Json::arrayValue);
    for (size_t col = 0; col < table->columnCount(); ++col) {
      fun.column = col;
      json_row.append(ts(table->typeOfColumn(col), fun));
    }
    rows.append(json_row);

  }
  return rows;
}

Json::Value generateRowsJson(const std::shared_ptr<const storage::AbstractTable>& table,
                             const size_t transmitLimit, const size_t transmitOffset) {
  if (const auto& store = std::dynamic_pointer_cast<const storage::SimpleStore>(table)) {
    return generateRowsJsonT(store, transmitLimit, transmitOffset);
  } else {
    return generateRowsJsonT(table, transmitLimit, transmitOffset);
  }
}

const std::string ResponseTask::vname() {
  return "ResponseTask";
}

void ResponseTask::registerPlanOperation(const std::shared_ptr<PlanOperation>& planOp) {
  performance_attributes_t* perf;
  std::vector<hyrise_int_t>* genKeys = new std::vector<hyrise_int_t>;

  if (_recordPerformanceData) {
    perf = new performance_attributes_t;
    planOp->setPerformanceData(perf);
  }
  
  planOp->setGeneratedKeysData(genKeys);

  const auto responseTaskPtr = std::dynamic_pointer_cast<ResponseTask>(shared_from_this());
  planOp->setResponseTask(responseTaskPtr);

  perfMutex.lock();

  if (_recordPerformanceData) {
    performance_data.push_back(std::unique_ptr<performance_attributes_t>(perf));
  }

  _generatedKeyRefs.push_back(std::unique_ptr<std::vector<hyrise_int_t>>(genKeys));
  perfMutex.unlock();
}


std::shared_ptr<PlanOperation> ResponseTask::getResultTask() {
  if (getDependencyCount() >  0) {
    return std::dynamic_pointer_cast<PlanOperation>(_dependencies[0]);
  }
  return nullptr;
}


task_states_t ResponseTask::getState() const {
  for (const auto& dep: _dependencies) {
    if (auto ot = std::dynamic_pointer_cast<OutputTask>(dep)) {
      if (ot->getState() != OpSuccess) return OpFail;
    }
  }
  return OpSuccess;
}

void ResponseTask::operator()() {
  epoch_t responseStart = _recordPerformanceData ? get_epoch_nanoseconds() : 0;
  Json::Value response;

  if (getDependencyCount() > 0) {
    PapiTracer pt;
    pt.addEvent("PAPI_TOT_CYC");

    if(_recordPerformanceData) pt.start();

    auto predecessor = getResultTask();
    const auto& result = predecessor->getResultTable();

    if (getState() != OpFail) {
      if (!_isAutoCommit) {
        response["session_context"] = std::to_string(_txContext.tid).append(" ").append(std::to_string(_txContext.lastCid));
      }

      if (result) {
        // Make header
        Json::Value json_header(Json::arrayValue);
        for (unsigned col = 0; col < result->columnCount(); ++col) {
          Json::Value colname(result->nameOfColumn(col));
          json_header.append(colname);
        }

        // Copy the complete result
        response["real_size"] = result->size();
        response["rows"] = generateRowsJson(result, _transmitLimit, _transmitOffset);
        response["header"] = json_header;
      }

      ////////////////////////////////////////////////////////////////////////////////////////
      // Copy Performance Data
      if (_recordPerformanceData) {
        Json::Value json_perf(Json::arrayValue);
        for (const auto & attr: performance_data) {
          Json::Value element;
          element["papi_event"] = Json::Value(attr->papiEvent);
          element["duration"] = Json::Value((Json::UInt64) attr->duration);
          element["data"] = Json::Value((Json::UInt64) attr->data);
          element["name"] = Json::Value(attr->name);
          element["id"] = Json::Value(attr->operatorId);
          element["startTime"] = Json::Value((double)(attr->startTime - queryStart) / 1000000);
          element["endTime"] = Json::Value((double)(attr->endTime - queryStart) / 1000000);
          element["executingThread"] = Json::Value(attr->executingThread);
          json_perf.append(element);
        }
        
        pt.stop();

        Json::Value responseElement;
        responseElement["duration"] = Json::Value((Json::UInt64) pt.value("PAPI_TOT_CYC"));
        responseElement["name"] = Json::Value("ResponseTask");
        responseElement["id"] = Json::Value("respond");
        responseElement["startTime"] = Json::Value((double)(responseStart - queryStart) / 1000000);
        responseElement["endTime"] = Json::Value((double)(get_epoch_nanoseconds() - queryStart) / 1000000);

        std::string threadId = boost::lexical_cast<std::string>(std::this_thread::get_id());
        responseElement["executingThread"] = Json::Value(threadId);
        json_perf.append(responseElement);

        response["performanceData"] = json_perf;
      }

      Json::Value jsonKeys(Json::arrayValue);
      for( const auto& x : _generatedKeyRefs) {
        for(const auto& key : *x) {
          Json::Value element(key);
          jsonKeys.append(element);
        }
      }
      response["generatedKeys"] = jsonKeys;
      response["affectedRows"] = Json::Value(_affectedRows);

    }
    LOG4CXX_DEBUG(_logger, "Table Use Count: " << result.use_count());
  }

  if (!_error_messages.empty()) {
    Json::Value errors;
    for (const auto& msg: _error_messages) {
      errors.append(Json::Value(msg));
    }
    response["error"] = errors;
  }

  LOG4CXX_DEBUG(_logger, response);

  Json::FastWriter fw;
  connection->respond(fw.write(response));
}

}
}
