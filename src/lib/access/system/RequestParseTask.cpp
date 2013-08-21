// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/RequestParseTask.h"

#include <array>
#include <map>
#include <string>
#include <vector>
#include <thread>

#include "boost/lexical_cast.hpp"

#include "access/system/ResponseTask.h"
#include "access/system/PlanOperation.h"
#include "access/system/QueryTransformationEngine.h"
#include "access/tx/Commit.h"

#include "helper/epoch.h"
#include "helper/HttpHelper.h"
#include "helper/numerical_converter.h"
#include "helper/PapiTracer.h"
#include "helper/sha1.h"

#include "io/TransactionManager.h"
#include "net/Router.h"
#include "net/AbstractConnection.h"

#include "taskscheduler/AbstractTaskScheduler.h"
#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace access {

bool registered = net::Router::registerRoute<RequestParseTask>(
    "/query/",
    net::Router::route_t::CATCH_ALL);

RequestParseTask::RequestParseTask(net::AbstractConnection* connection)
    : _connection(connection),
      _responseTask(std::make_shared<ResponseTask>(connection)),
      _queryStart(get_epoch_nanoseconds()){}

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

  performance_vector_t& performance_data = _responseTask->getPerformanceData();
  // the performance attribute for this operation (at [0])
  performance_data.push_back(std::unique_ptr<performance_attributes_t>(new performance_attributes_t));

  std::vector<std::shared_ptr<Task> > tasks;

  int priority = Task::DEFAULT_PRIORITY;
  int sessionId = 0;

  if (_connection->hasBody()) {
    // The body is a wellformed HTTP Post body, with key value pairs
    std::string body(_connection->getBody());
    std::map<std::string, std::string> body_data = parseHTTPFormData(body);

    boost::optional<tx::TXContext> ctx;
    auto ctx_it = body_data.find("session_context");
    if (ctx_it != body_data.end()) {
      boost::optional<tx::transaction_id_t> tid;
      if ((tid = parseNumeric<tx::transaction_id_t>(ctx_it->second)) &&
          (tx::TransactionManager::isRunningTransaction(*tid))) {
        LOG4CXX_DEBUG(_logger, "Picking up transaction id " << *tid);
        ctx = tx::TransactionManager::getContext(*tid);
      } else {
        LOG4CXX_ERROR(_logger, "Invalid transaction id " << *tid);
        _responseTask->addErrorMessage("Invalid transaction id set, aborting execution.");
      }
    } else {
      ctx = tx::TransactionManager::beginTransaction();
      LOG4CXX_DEBUG(_logger, "Creating new transaction context " << (*ctx).tid);
    }

    Json::Value request_data;
    Json::Reader reader;

    if (ctx && reader.parse(urldecode(body_data["query"]), request_data)) {
      _responseTask->setTxContext(*ctx);

      LOG4CXX_DEBUG(_query_logger, request_data);

      std::string final_hash = hash(request_data);
      std::shared_ptr<Task> result = nullptr;

      if(request_data.isMember("priority"))
        priority = request_data["priority"].asInt();
      if(request_data.isMember("sessionId"))
        sessionId = request_data["sessionId"].asInt();
      _responseTask->setPriority(priority);
      _responseTask->setSessionId(sessionId);
      try {
        tasks = QueryParser::instance().deserialize(
                  QueryTransformationEngine::getInstance()->transform(request_data),
                  &result);

      } catch (const std::exception &ex) {
        // clean up, so we don't end up with a whole mess due to thrown exceptions
        LOG4CXX_ERROR(_logger, "Received\n:" << request_data);
        LOG4CXX_ERROR(_logger, "Exception thrown during query deserialization:\n" << ex.what());
        _responseTask->addErrorMessage(std::string("RequestParseTask: ") + ex.what());
        tasks.clear();
        result = nullptr;
      }

      auto autocommit_it = body_data.find("autocommit");
      if (autocommit_it != body_data.end() && (autocommit_it->second == "true")) {
        auto commit = std::make_shared<Commit>();
        commit->setOperatorId("__autocommit");
        commit->setPlanOperationName("Commit");
        commit->addDependency(result);
        result = commit;
        tasks.push_back(commit);
      }


      if (result != nullptr) {
        _responseTask->addDependency(result);
      } else {
        LOG4CXX_ERROR(_logger, "Json did not yield tasks");
      }

      for (const auto & func: tasks) {
        if (auto task = std::dynamic_pointer_cast<PlanOperation>(func)) {
          task->setPriority(priority);
          task->setSessionId(sessionId);
          task->setPlanId(final_hash);
          task->setTXContext(*ctx);
	  task->setId((*ctx).tid);
	  _responseTask->registerPlanOperation(task);
          if (!task->hasSuccessors()) {
            // The response has to depend on all tasks, ie. we don't
            // want to respond before all tasks finished running, even
            // if they don't contribute to the result. This prevents
            // dangling tasks
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

    if (atoi(body_data["offset"].c_str()) > 0)
      _responseTask->setTransmitOffset(atol(body_data["offset"].c_str()));

  } else {
    LOG4CXX_WARN(_logger, "no body received!");
  }




  // high priority tasks are expected to be scheduled sequentially
  if(priority == Task::HIGH_PRIORITY){
    *(performance_data.at(0)) = { 0, 0, "NO_PAPI", "RequestParseTask", "requestParse", _queryStart, get_epoch_nanoseconds(), boost::lexical_cast<std::string>(std::this_thread::get_id()) };
    int number_of_tasks = tasks.size();
    std::vector<bool> isExecuted(number_of_tasks, false);
    int executedTasks = 0;
    while(executedTasks < number_of_tasks){
      for(int i = 0; i < number_of_tasks; i++){
        if(!isExecuted[i] && tasks[i]->isReady()){
          (*tasks[i])();
          tasks[i]->notifyDoneObservers();
          executedTasks++;
          isExecuted[i] = true;
        }
      }
    }
    _responseTask->setQueryStart(_queryStart);
    (*_responseTask)();
    _responseTask.reset();  // yield responsibility

  } else {
    scheduler->scheduleQuery(tasks);
    *(performance_data.at(0)) = { 0, 0, "NO_PAPI", "RequestParseTask", "requestParse", _queryStart, get_epoch_nanoseconds(), boost::lexical_cast<std::string>(std::this_thread::get_id()) };
    _responseTask->setQueryStart(_queryStart);
    scheduler->schedule(_responseTask);
    _responseTask.reset();  // yield responsibility
  }
}

std::shared_ptr<ResponseTask> RequestParseTask::getResponseTask() const {
  return _responseTask;
}

}
}
