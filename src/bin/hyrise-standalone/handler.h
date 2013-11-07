#ifndef HYRISE_STANDALONE_HANDLER_H_
#define HYRISE_STANDALONE_HANDLER_H_

#include <iostream>
#include <boost/lexical_cast.hpp>

#include "taskscheduler/SharedScheduler.h"

#include "access/system/ResponseTask.h"
#include "access/tx/Commit.h"
#include "access/system/PlanOperation.h"
#include "access/system/QueryTransformationEngine.h"

#include "helper/epoch.h"
#include "helper/HttpHelper.h"
#include "helper/numerical_converter.h"
#include "helper/PapiTracer.h"
#include "helper/sha1.h"

#include "helper/HwlocHelper.h"
#include "net/ThriftConnection.h"
#include "io/StorageManager.h"
#include "io/TransactionManager.h"
#include "taskscheduler/SharedScheduler.h"

#include "net/ThriftConnection.h"

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

using namespace hyrise;
using namespace log4cxx;
using namespace log4cxx::helpers;


namespace {
  LoggerPtr slogger(Logger::getLogger("hyrise"));
  //log4cxx::LoggerPtr _query_logger(log4cxx::Logger::getLogger("hyrise.access.queries"));

}

class HyriseHandler {

  std::string _schedulerName;
  int _workerThreads;

  AbstractTaskScheduler* scheduler = nullptr;

  
  // Hash The input to a small string
  std::string hash(const std::string &v) {
    const std::string& jsonData = v;

    std::array<unsigned char, 20> hash;
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, (const unsigned char *) jsonData.c_str(), jsonData.size());
    SHA1Final(hash.data(), &ctx);

    return std::string(reinterpret_cast<const char*>(hash.data()), 20);
  }

public:

  HyriseHandler(std::string schedName, int workerThreads) : _schedulerName(schedName), _workerThreads(workerThreads) {}

  static std::string loadFromFile(std::string path) {
    std::ifstream data_file(path.c_str());
    std::string result((std::istreambuf_iterator<char>(data_file)), std::istreambuf_iterator<char>());
    data_file.close();
    return result;
  }
  

  void init() {
    SharedScheduler::getInstance().init(_schedulerName, _workerThreads);
    scheduler = SharedScheduler::getInstance().getScheduler();
  }

  std::string execute(const std::string& query_string) {
    // Build the response task
    epoch_t _queryStart;
    auto conn = new hyrise::net::ThriftConnection(query_string);
    auto _responseTask = std::make_shared<hyrise::access::ResponseTask>(conn);
    hyrise::access::performance_vector_t& performance_data = _responseTask->getPerformanceData();

    bool recordPerformance = false;
    std::vector<std::shared_ptr<Task> > tasks;

    int priority = Task::DEFAULT_PRIORITY;
    int sessionId = 0;

    // Recovering older sessions
    tx::TXContext ctx = hyrise::tx::TransactionManager::beginTransaction();

    // JSON Parsing
    Json::Value request_data;
    Json::Reader reader;

    // Parsing the request
    if (reader.parse(query_string, request_data)) {
      _responseTask->setTxContext(ctx);

      // Modify Performance data 
      recordPerformance = request_data["performance"].asBool();
      _responseTask->setRecordPerformanceData(recordPerformance);
      
      // the performance attribute for this operation (at [0])
      if (recordPerformance) {
        _queryStart = get_epoch_nanoseconds();
        performance_data.push_back(std::unique_ptr<hyrise::access::performance_attributes_t>(new hyrise::access::performance_attributes_t));
      }

      LOG4CXX_DEBUG(slogger, request_data);

      const std::string& final_hash = "";//hash(query_string);
      std::shared_ptr<Task> result = nullptr;

      if(request_data.isMember("priority"))
        priority = request_data["priority"].asInt();
      if(request_data.isMember("sessionId"))
        sessionId = request_data["sessionId"].asInt();
      _responseTask->setPriority(priority);
      _responseTask->setSessionId(sessionId);
      try {
        tasks = hyrise::access::QueryParser::instance().deserialize(
                  QueryTransformationEngine::getInstance()->transform(request_data),
                  &result);

      } catch (const std::exception &ex) {
        // clean up, so we don't end up with a whole mess due to thrown exceptions
        LOG4CXX_ERROR(slogger, "Received\n:" << request_data);
        LOG4CXX_ERROR(slogger, "Exception thrown during query deserialization:\n" << ex.what());
        _responseTask->addErrorMessage(std::string("RequestParseTask: ") + ex.what());
        tasks.clear();
        result = nullptr;
      }

      // Append autocommit commit operation
      if (false) {
        auto commit = std::make_shared<hyrise::access::Commit>();
        commit->setOperatorId("__autocommit");
        commit->setPlanOperationName("Commit");
        commit->addDependency(result);
        result = commit;
        tasks.push_back(commit);
      }


      if (result != nullptr) {
        _responseTask->addDependency(result);
      } else {
        LOG4CXX_ERROR(slogger, "Json did not yield tasks");
      }

      for (const auto & func: tasks) {
        if (auto task = std::dynamic_pointer_cast<hyrise::access::PlanOperation>(func)) {
          task->setPriority(priority);
          task->setSessionId(sessionId);
          task->setPlanId(final_hash);
          task->setTXContext(ctx);
          task->setId((ctx).tid);
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
      LOG4CXX_WARN(slogger, "no body received!");
    }

    // Now schedule all tasks and fix it
    // high priority tasks are expected to be scheduled sequentially
    if(priority == Task::HIGH_PRIORITY){
      if (recordPerformance) {
        *(performance_data.at(0)) = { 0, 0, "NO_PAPI", "RequestParseTask", 
        "requestParse", _queryStart, get_epoch_nanoseconds(), 
        boost::lexical_cast<std::string>(std::this_thread::get_id()) };
      }

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

      // Finish the query
      _responseTask->setQueryStart(_queryStart);
      (*_responseTask)();
      auto result = conn->getResponse();
      delete conn;      
      return result;

    } else {

      // PRepare the wait task
      auto wait = std::make_shared<WaitTask>();
      wait->addDependency(_responseTask);
      scheduler->schedule(wait);
      scheduler->schedule(_responseTask);
      scheduler->scheduleQuery(tasks);

      if (recordPerformance) {
        *(performance_data.at(0)) = { 0, 0, "NO_PAPI", "RequestParseTask", "requestParse", 
                                      _queryStart, get_epoch_nanoseconds(), 
                                      boost::lexical_cast<std::string>(std::this_thread::get_id()) };
      }
      _responseTask->setQueryStart(_queryStart);
      wait->wait();
      auto result = conn->getResponse();
      delete conn;
      return result;
    }

  }

};

#endif // HYRISE_STANDALONE_HANDLER_H_