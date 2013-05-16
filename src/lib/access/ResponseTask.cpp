// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ResponseTask.h"

#include <thread>

#include "json.h"
#include "log4cxx/logger.h"
#include "boost/lexical_cast.hpp"

#include "access/PlanOperation.h"
#include "helper/PapiTracer.h"
#include "net/AsyncConnection.h"
#include "storage/AbstractTable.h"
#include "storage/SimpleStore.h"
#include "storage/meta_storage.h"
#include "access/OutputTask.h"

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
Json::Value generateRowsJsonT(const T& table, const size_t transmitLimit) {
  hyrise::storage::type_switch<hyrise_basic_types> ts;
  json_functor<T> fun(table);
  Json::Value rows(Json::arrayValue);
  for (size_t row = 0; row < table->size(); ++row) {

    fun.row = row;
    Json::Value json_row(Json::arrayValue);
    for (size_t col = 0; col < table->columnCount(); ++col) {
      fun.column = col;
      json_row.append(ts(table->typeOfColumn(col), fun));
    }
    rows.append(json_row);

    if (transmitLimit > 0 && row == transmitLimit)
      break;
  }
  return rows;
}

Json::Value generateRowsJson(const std::shared_ptr<const AbstractTable>& table,
                             const size_t transmitLimit) {
  if (const auto& store = std::dynamic_pointer_cast<const hyrise::storage::SimpleStore>(table)) {
      return generateRowsJsonT(store, transmitLimit);
  } else {
      return generateRowsJsonT(table, transmitLimit);
  }
}

const std::string ResponseTask::vname() {
  return "ResponseTask";
}

void ResponseTask::registerPlanOperation(const std::shared_ptr<_PlanOperation>& planOp) {
  OutputTask::performance_attributes_t* perf = new OutputTask::performance_attributes_t;
  planOp->setPerformanceData(perf);

  const auto responseTaskPtr = std::dynamic_pointer_cast<ResponseTask>(shared_from_this()); 
  planOp->setResponseTask(responseTaskPtr);

  perfMutex.lock();
  performance_data.push_back(std::unique_ptr<OutputTask::performance_attributes_t>(perf));
  perfMutex.unlock();
}


std::shared_ptr<_PlanOperation> ResponseTask::getResultTask() {
  if (getDependencyCount() > 0) {
    return std::dynamic_pointer_cast<_PlanOperation>(_dependencies[0]);
  }
  return nullptr;
}

void ResponseTask::operator()() {
  epoch_t responseStart = get_epoch_nanoseconds();
  Json::Value response;

  if (getDependencyCount() > 0) {
    PapiTracer pt;
    pt.addEvent("PAPI_TOT_CYC");
    pt.start();

    auto predecessor = getResultTask();
    const auto& result = predecessor->getResultTable();

    if (predecessor->getState() != OpFail) {
      if (result) {
        // Make header
        Json::Value json_header(Json::arrayValue);
        for (unsigned col = 0; col < result->columnCount(); ++col) {
          Json::Value colname(result->nameOfColumn(col));
          json_header.append(colname);
        }

        // Copy the complete result
        response["real_size"] = result->size();
        response["rows"] = generateRowsJson(result, _transmitLimit);
        response["header"] = json_header;
      }

      // Copy Performance Data
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
    } else {
      LOG4CXX_ERROR(_logger, "Error during plan execution: " << predecessor->getErrorMessage());
      response["error"] = predecessor->getErrorMessage();
    }
    LOG4CXX_DEBUG(_logger, "Table Use Count: " << result.use_count());
  } else {
    response["error"] = "Query parsing failed, see server error log";
  }

  connection->respond(response.toStyledString());
}

}
}
