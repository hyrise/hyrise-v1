// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/RequestParseTask.h"

#include <array>
#include <iomanip>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <thread>

#include "boost/lexical_cast.hpp"

#include "access/system/ResponseTask.h"
#include "access/system/PlanOperation.h"
#include "access/system/QueryTransformationEngine.h"
#include "access/tx/Commit.h"
#include "access/sql/SQLQueryParser.h"

#include "helper/epoch.h"
#include "helper/HttpHelper.h"
#include "helper/numerical_converter.h"
#include "helper/PapiTracer.h"
#include "helper/sha1.h"
#include "helper/vector_helpers.h"
#include "io/TransactionManager.h"
#include "net/Router.h"
#include "net/AbstractConnection.h"

#include "taskscheduler/AbstractTaskScheduler.h"
#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace access {

bool registered = net::Router::registerRoute<RequestParseTask>("/query/", net::Router::route_t::CATCH_ALL);

RequestParseTask::RequestParseTask(net::AbstractConnection* connection)
    : _connection(connection),
      _responseTask(std::make_shared<ResponseTask>(connection)),
      _queryStart(get_epoch_nanoseconds()) {
  connection->setResponseTask(_responseTask);
}

RequestParseTask::~RequestParseTask() {}

std::string RequestParseTask::name() { return "RequestParseTask"; }

const std::string RequestParseTask::vname() { return "RequestParseTask"; }

namespace {
log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access"));
log4cxx::LoggerPtr _query_logger(log4cxx::Logger::getLogger("hyrise.access.queries"));
}

std::string hash(const std::string& v) {
  const std::string& jsonData = v;

  std::array<unsigned char, 20> hash;
  SHA1_CTX ctx;
  SHA1Init(&ctx);
  SHA1Update(&ctx, (const unsigned char*)jsonData.c_str(), jsonData.size());
  SHA1Final(hash.data(), &ctx);

  return std::string(reinterpret_cast<const char*>(hash.data()), 20);
}

void RequestParseTask::operator()() {
  auto opStart = get_epoch_nanoseconds();
  assert((_responseTask != nullptr) && "Response needs to be set");
  std::shared_ptr<hyrise::taskscheduler::AbstractTaskScheduler> scheduler;
  if (_scheduler)
    scheduler = _scheduler;
  else
    scheduler = taskscheduler::SharedScheduler::getInstance().getScheduler();

  performance_vector_t& performance_data = _responseTask->getPerformanceData();

  bool recordPerformance = false;
  bool autocommit = false;
  std::vector<std::shared_ptr<Task> > tasks;

  int priority = Task::DEFAULT_PRIORITY;
  int sessionId = 0;

  if (_connection->hasBody()) {
    // The body is a wellformed HTTP Post body, with key value pairs
    std::string body(_connection->getBody());
    std::map<std::string, std::string> body_data = parseHTTPFormData(body);

    tx::TXContext ctx;
    auto ctx_it = body_data.find("session_context");
    if (ctx_it != body_data.end()) {
      std::size_t pos;
      tx::transaction_id_t tid = std::stoll(ctx_it->second.c_str(), &pos);
      tx::transaction_id_t cid = std::stoll(ctx_it->second.c_str() + pos + 1, &pos);
      ctx = tx::TXContext(tid, cid);
    } else {
      ctx = tx::TransactionManager::beginTransaction();
      LOG4CXX_DEBUG(_logger, "Creating new transaction context " << ctx.tid);
    }

    // Query parsing
    // SQL Query variables
    bool is_sql_query = false;
    std::string sql_query;

    // JSON Query variables
    bool is_json_query = false;
    std::string query_string = "";
    Json::Value json_request_data;
    Json::Reader reader;

    // indicates the query was successfully parsed
    bool parse_was_successful = false;

    if (body_data.find("sql") != body_data.end()) {
      is_sql_query = true;
      sql_query = urldecode(body_data["sql"]);
      parse_was_successful = true;

    } else if (body_data.find("query") != body_data.end()) {
      is_json_query = true;
      query_string = urldecode(body_data["query"]);
      parse_was_successful = reader.parse(query_string, json_request_data);
    }


    // Evaluate the parsed query
    if (parse_was_successful) {
      recordPerformance = getOrDefault(body_data, "performance", "false") == "true";

      // the performance attribute for this operation (at [0])
      if (recordPerformance) {
        performance_data.push_back(std::unique_ptr<performance_attributes_t>(new performance_attributes_t));
      }

      const std::string& final_hash = hash(query_string);
      std::shared_ptr<Task> result = nullptr;

      
      if (is_json_query) {
        LOG4CXX_DEBUG(_query_logger, json_request_data);

        if (json_request_data.isMember("priority"))
          priority = json_request_data["priority"].asInt();
        if (json_request_data.isMember("sessionId"))
          sessionId = json_request_data["sessionId"].asInt();

        auto autocommit_it = body_data.find("autocommit");
        autocommit = (autocommit_it != body_data.end() && (autocommit_it->second == "true"));
      } // json specific block

      if (is_sql_query) {
        LOG4CXX_DEBUG(_query_logger, sql_query);
        
      } // sql specific block

      _responseTask->setTxContext(ctx);
      _responseTask->setPriority(priority);
      _responseTask->setSessionId(sessionId);
      _responseTask->setRecordPerformanceData(recordPerformance);


      try {
        // Generate tasks from query
        if (is_json_query) {
          tasks = QueryParser::instance().deserialize(
                    QueryTransformationEngine::getInstance()->transform(json_request_data),
                    &result);

        } else if (is_sql_query) {

          sql::SQLQueryParser parser(sql_query, _responseTask);
          tasks = parser.buildSQLQueryTasks();
          result = tasks.back();
          
        }

      } catch (const std::exception& ex) {

        // clean up, so we don't end up with a whole mess due to thrown exceptions
        if (is_json_query) LOG4CXX_ERROR(_logger, "Received\n:" << json_request_data);
        if (is_sql_query) LOG4CXX_ERROR(_logger, "Received\n:" << sql_query);
        LOG4CXX_ERROR(_logger, "Exception thrown during query deserialization:\n" << ex.what());
        _responseTask->addErrorMessage(std::string("RequestParseTask: ") + ex.what());
        tasks.clear();
        result = nullptr;
      }


      /**
       * Autocommit at the end of query if specified
       */
#ifdef PERSISTENCY_BUFFEREDLOGGER
      auto group_commit_it = body_data.find("autocommit");
      bool group_commit = (group_commit_it != body_data.end() && (group_commit_it->second == "true"));
      _responseTask->setGroupCommit(group_commit);
#endif

      if (autocommit) {
        auto commit = std::make_shared<Commit>();
        commit->setOperatorId("__autocommit");
        commit->setPlanOperationName("Commit");
        commit->addDependency(result);

#ifdef PERSISTENCY_BUFFEREDLOGGER
        commit->setFlushLog(!group_commit);
#endif
        
        result = commit;
        tasks.push_back(commit);
        _responseTask->setIsAutoCommit(true);
      }


      if (tasks.size() == 0) {
        LOG4CXX_ERROR(_logger, "Parsing of query did not yield result task");
      }

      if (result != nullptr) {
        _responseTask->addDependency(result);
      }

      for (const auto& func : tasks) {
        if (auto task = std::dynamic_pointer_cast<PlanOperation>(func)) {
          task->setPriority(priority);
          task->setSessionId(sessionId);
          task->setPlanId(final_hash);
          task->setTXContext(ctx);
          task->setId(ctx.tid);
          _responseTask->registerPlanOperation(task);
          if (!task->hasSuccessors()) {
            // The response has to depend on all tasks, ie. we don't
            // want to respond before all tasks finished running, even
            // if they don't contribute to the result. This prevents
            // dangling tasks
            _responseTask->addDependency(task);
          }
        }
#ifdef PERSISTENCY_BUFFEREDLOGGER
        if (auto commit = std::dynamic_pointer_cast<Commit>(func)) {
          commit->setFlushLog(!group_commit);
        }
#endif
      }
    } else {
      LOG4CXX_ERROR(_logger,
                    "Failed to parse: " << urldecode(body_data["query"]) << "\n" << body_data["query"] << "\n"
                                        << reader.getFormatedErrorMessages());

      // Forward parsing error
      _responseTask->addErrorMessage("Parsing: " + reader.getFormatedErrorMessages());
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
  if (priority == Task::HIGH_PRIORITY) {
    if (recordPerformance) {
      *(performance_data.at(0)) = {
          0,                                 0,
          "NO_PAPI",                         "RequestParseTask",
          "requestParse",                    opStart,
          get_epoch_nanoseconds(),           boost::lexical_cast<std::string>(std::this_thread::get_id()),
          std::numeric_limits<size_t>::max()};
    }

    int number_of_tasks = tasks.size();
    std::vector<bool> isExecuted(number_of_tasks, false);
    int executedTasks = 0;
    while (executedTasks < number_of_tasks) {
      for (int i = 0; i < number_of_tasks; i++) {
        if (!isExecuted[i] && tasks[i]->isReady()) {
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
    scheduler->schedule(_responseTask);
    scheduler->scheduleQuery(tasks);

    if (recordPerformance) {
      *(performance_data.at(0)) = {
          0,                                 0,
          "NO_PAPI",                         "RequestParseTask",
          "requestParse",                    opStart,
          get_epoch_nanoseconds(),           boost::lexical_cast<std::string>(std::this_thread::get_id()),
          std::numeric_limits<size_t>::max()};
    }
    _responseTask->setQueryStart(_queryStart);
    _responseTask.reset();  // yield responsibility
  }
}

std::shared_ptr<ResponseTask> RequestParseTask::getResponseTask() const { return _responseTask; }
}
}
