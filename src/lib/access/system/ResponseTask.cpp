// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/ResponseTask.h"

#include <thread>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

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

  typedef void value_type;

  const T& table;
  size_t column;
  size_t row;

  // Used for building the result
  rapidjson::Document& doc;
  size_t pos = 0;

  explicit json_functor(const T& t, rapidjson::Document& d): table(t), column(0), row(0), doc(d){}


  void beginRow() {
    rapidjson::Value v(rapidjson::kArrayType);
    doc.PushBack(v, doc.GetAllocator());
    pos = doc.size() - 1;
  }

  template <typename R>
  void operator()() {
    doc[pos].PushBack(table->template getValue<R>(column, row), doc.GetAllocator());
  }
};

template<typename T>
rapidjson::Document&& generateRowsJsonT(const T& table, const size_t transmitLimit, const size_t transmitOffset) {
  hyrise::storage::type_switch<hyrise_basic_types> ts;

  rapidjson::Document rows;
  rows.SetArray();
  json_functor<T> fun(table, rows);

  for (size_t row = 0; row < table->size(); ++row) {

    // Align offset
    if (row < transmitOffset)
      continue;

    // Break if limit reached
    if (transmitLimit > 0 && row == (transmitOffset + transmitLimit))
      break;

    fun.row = row;
    fun.beginRow();
    for (size_t col = 0; col < table->columnCount(); ++col) {
      fun.column = col;
      ts(table->typeOfColumn(col), fun);
    }
  }
  return std::move(rows);
}

rapidjson::Document&& generateRowsJson(const std::shared_ptr<const AbstractTable>& table,
                             const size_t transmitLimit, const size_t transmitOffset) {

  if (const auto& store = std::dynamic_pointer_cast<const hyrise::storage::SimpleStore>(table)) {
    return generateRowsJsonT(store, transmitLimit, transmitOffset);
  } else {
    return generateRowsJsonT(table, transmitLimit, transmitOffset);
  }
}

const std::string ResponseTask::vname() {
  return "ResponseTask";
}

void ResponseTask::registerPlanOperation(const std::shared_ptr<PlanOperation>& planOp) {
  performance_attributes_t* perf = new performance_attributes_t;
  std::vector<hyrise_int_t>* genKeys = new std::vector<hyrise_int_t>;

  planOp->setPerformanceData(perf);
  planOp->setGeneratedKeysData(genKeys);

  const auto responseTaskPtr = std::dynamic_pointer_cast<ResponseTask>(shared_from_this());
  planOp->setResponseTask(responseTaskPtr);

  perfMutex.lock();
  performance_data.push_back(std::unique_ptr<performance_attributes_t>(perf));
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
  epoch_t responseStart = get_epoch_nanoseconds();
  rapidjson::Document response;

  if (getDependencyCount() > 0) {
    PapiTracer pt;
    pt.addEvent("PAPI_TOT_CYC");
    pt.start();

    auto predecessor = getResultTask();
    const auto& result = predecessor->getResultTable();

    if (getState() != OpFail) {

      if (tx::TransactionManager::isRunningTransaction(_txContext.tid)) {
        response.AddMember("session_context", _txContext.tid, response.GetAllocator());
      }

      if (result) {
        // Make header
        rapidjson::Value json_header(rapidjson::kArrayType);
        for (unsigned col = 0; col < result->columnCount(); ++col) {
          json_header.PushBack(result->nameOfColumn(col), response.GetAllocator());
        }

        // Copy the complete result
        response["real_size"] = result->size();
        rapidjson::Document tmp_rows(generateRowsJson(result, _transmitLimit, _transmitOffset));
        response.AddMember("rows", tmp_rows, response.GetAllocator());
        response["header"] = json_header;
      }

      // Copy Performance Data
      rapidjson::Value json_perf(rapidjson::kArrayType);
      for (const auto & attr: performance_data) {
        rapidjson::Value element(rapidjson::kObjectType);
        element.AddMember("papi_event", attr->papiEvent, response.GetAllocator());
        element.AddMember("duration", attr->duration, response.GetAllocator());
        element.AddMember("data", attr->data, response.GetAllocator());
        element.AddMember("name", attr->name, response.GetAllocator());
        element.AddMember("id", attr->operatorId, response.GetAllocator());
        element.AddMember("startTime", (double)(attr->startTime - queryStart) / 1000000, response.GetAllocator());
        element.AddMember("endTime", (double)(attr->endTime - queryStart) / 1000000, response.GetAllocator());
        element.AddMember("executingThread", attr->executingThread, response.GetAllocator());
        json_perf.PushBack(element, response.GetAllocator());
      }
      pt.stop();

      rapidjson::Value responseElement(rapidjson::kObjectType);
      responseElement.AddMember("duration", (uint64_t) pt.value("PAPI_TOT_CYC"), response.GetAllocator());
      responseElement.AddMember("name", "ResponseTask", response.GetAllocator());
      responseElement.AddMember("id", "respond", response.GetAllocator());
      responseElement.AddMember("startTime", (double)(responseStart - queryStart) / 1000000, response.GetAllocator());
      responseElement.AddMember("endTime", (double)(get_epoch_nanoseconds() - queryStart) / 1000000, response.GetAllocator());
      responseElement.AddMember("executingThread", boost::lexical_cast<std::string>(std::this_thread::get_id()) , response.GetAllocator());
      json_perf.PushBack(responseElement, response.GetAllocator());
                                
      response.AddMember("performanceData", json_perf, response.GetAllocator());

      rapidjson::Value jsonKeys(rapidjson::kArrayType);
      for( const auto& x : _generatedKeyRefs) {
        for(const auto& key : *x) {
          jsonKeys.PushBack(key, response.GetAllocator());
        }
      }
      response.AddMember("generatedKeys", jsonKeys, response.GetAllocator());
      response.AddMember("affectedRows", _affectedRows.load(), response.GetAllocator());

    }
    LOG4CXX_DEBUG(_logger, "Table Use Count: " << result.use_count());
  }

  if (!_error_messages.empty()) {
    rapidjson::Value errors(rapidjson::kArrayType);
    for (const auto& msg: _error_messages) {
      errors.PushBack(msg, response.GetAllocator());
    }
    response.AddMember("error", errors, response.GetAllocator());
  }

  rapidjson::StringBuffer wss;
  rapidjson::Writer<decltype(wss)> w(wss);
  response.Accept(w);
  connection->respond(wss.GetString());
}

}
}
