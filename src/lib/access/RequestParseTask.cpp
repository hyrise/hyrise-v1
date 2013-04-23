// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/RequestParseTask.h"

#include <array>
#include <map>
#include <string>
#include <vector>
#include <thread>

#include "boost/lexical_cast.hpp"

#include "access/ResponseTask.h"
#include "access/PlanOperation.h"
#include "access/QueryTransformationEngine.h"
#include "helper/HttpHelper.h"
#include "helper/PapiTracer.h"
#include "helper/sha1.h"
#include "io/TransactionManager.h"
#include "net/Router.h"
#include "net/AbstractConnection.h"
#include "taskscheduler/AbstractTaskScheduler.h"


namespace hyrise {
namespace access {

bool registered = net::Router::registerRoute<RequestParseTask>(
    "/query/",
    net::Router::route_t::CATCH_ALL);

RequestParseTask::RequestParseTask(net::AbstractConnection* connection)
    : _connection(connection),
      _responseTask(std::make_shared<ResponseTask>(connection)) {}

RequestParseTask::~RequestParseTask() {}

std::string RequestParseTask::name() {
  return "RequestParseTask";
}

const std::string RequestParseTask::vname() {
  return "RequestParseTask";
}

namespace {
log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access"));
log4cxx::LoggerPtr _query_logger(log4cxx::Logger::getLogger("hyrise.access.queries"));
}

std::string hash(const Json::Value &v) {
  const std::string jsonData(v.toStyledString());

  std::array<unsigned char, 20> hash;
  SHA1_CTX ctx;
  SHA1Init(&ctx);
  SHA1Update(&ctx, (const unsigned char *) jsonData.c_str(), jsonData.size());
  SHA1Final(hash.data(), &ctx);

  return std::string(reinterpret_cast<const char*>(hash.data()), 20);
}

void RequestParseTask::operator()() {
  assert((_responseTask != nullptr) && "Response needs to be set");
  AbstractTaskScheduler *scheduler = SharedScheduler::getInstance().getScheduler();
  // MG response always on the same core
  _responseTask->setPreferredCore(0);

  OutputTask::performance_vector& performance_data = _responseTask->getPerformanceData();
  performance_data.resize(1); // make room for at leas *this* operator
  std::vector<std::shared_ptr<Task> > tasks;

  if (_connection->hasBody()) {
    // The body is a wellformed HTTP Post body, with key value pairs
    std::string body(_connection->getBody());
    std::map<std::string, std::string> body_data = parseHTTPFormData(body);

    Json::Value request_data;
    Json::Reader reader;
    if (reader.parse(urldecode(body_data["query"]), request_data)) {
      LOG4CXX_DEBUG(_query_logger, request_data);
      std::string final_hash = hash(request_data);
      std::shared_ptr<Task> result = nullptr;
      int priority;
      if(request_data.isMember("priority"))
        priority = request_data["priority"].asInt();
      else
        priority = Task::DEFAULT_PRIORITY;
      _responseTask->setPriority(priority);
      try {
        tasks = QueryParser::instance().deserialize(
                  QueryTransformationEngine::getInstance()->transform(request_data),
                  &result);

      } catch (const std::exception &ex) {
        // clean up, so we don't end up with a whole mess due to thrown exceptions
        LOG4CXX_ERROR(_logger, "Received\n:" << request_data);
        LOG4CXX_ERROR(_logger, "Exception thrown during query deserialization:\n" << ex.what());
        tasks.clear();
        result = nullptr;
      }

      if (result != nullptr) {
        _responseTask->addDependency(result);
      } else {
        LOG4CXX_ERROR(_logger, "Json did not yield tasks");
      }

      tx::transaction_id_t tid = tx::TransactionManager::getInstance().getTransactionId();
      OutputTask::performance_vector& performance_data = _responseTask->getPerformanceData();

      // We need space for *this* operator, too
      performance_data.resize(tasks.size() + 1);
      size_t i = 1;
      for (const auto & func: tasks) {
        if (auto task = std::dynamic_pointer_cast<_PlanOperation>(func)) {
          task->setPriority(priority);
          task->setPlanId(final_hash);
          task->setTransactionId(tid);
          task->setPerformanceData(&(performance_data.at(i++)));
          if (!task->hasSuccessors()) {
            // The response has to depend on all tasks, ie. we don't want to respond
            // before all tasks finished running, even if they don't contribute to the result
            // This prevents dangling tasks
            _responseTask->addDependency(task);
          }
        }
      }
    } else {
      LOG4CXX_ERROR(_logger, "Failed to parse: "
                    << urldecode(body_data["query"]) << "\n"
                    << body_data["query"] << "\n"
                    << reader.getFormatedErrorMessages());
    }
    // Update the transmission limit for the response task
    if (atoi(body_data["limit"].c_str()) > 0)
      _responseTask->setTransmitLimit(atol(body_data["limit"].c_str()));
  } else {
    LOG4CXX_WARN(_logger, "no body received!");
  }

  for (const auto& task: tasks) {
    scheduler->schedule(task);
  }

  performance_data[0] = { 0, 0, "NO_PAPI", "RequestParseTask", "requestParse", _queryStart, get_epoch_nanoseconds(), boost::lexical_cast<std::string>(std::this_thread::get_id()) };
  _responseTask->setQueryStart(_queryStart);
  scheduler->schedule(_responseTask);
  _responseTask.reset();  // yield responsibility
}

std::shared_ptr<ResponseTask> RequestParseTask::getResponseTask() const {
  return _responseTask;
}

}
}
