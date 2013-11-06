// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <hwloc.h>
#include <signal.h>


// Thrift Protocol
#include "Hyrise.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

#include <boost/program_options.hpp>

// Hyrise
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


namespace po = boost::program_options;

using namespace hyrise;
using namespace log4cxx;
using namespace log4cxx::helpers;

namespace  {

const char *PID_FILE = "./hyrise_server.pid";
const size_t DEFAULT_PORT = 5000;

LoggerPtr logger(Logger::getLogger("hyrise"));
log4cxx::LoggerPtr _query_logger(log4cxx::Logger::getLogger("hyrise.access.queries"));

}

class HyriseHandler : virtual public HyriseIf {
  AbstractTaskScheduler *scheduler;


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
  HyriseHandler() {
    scheduler = SharedScheduler::getInstance().getScheduler();
  }

  void ping() {
    // Your implementation goes here
    LOG4CXX_INFO(logger, "Ping to server...");
  }

  // Parse the actual query
  void query(Result& _return, const Query& q) {

    // Build the response task
    auto _queryStart = get_epoch_nanoseconds();
    auto conn = new hyrise::net::ThriftConnection(q.query);
    auto _responseTask = std::make_shared<hyrise::access::ResponseTask>(conn);
    hyrise::access::performance_vector_t& performance_data = _responseTask->getPerformanceData();

    bool recordPerformance = false;
    std::vector<std::shared_ptr<Task> > tasks;

    int priority = Task::DEFAULT_PRIORITY;
    int sessionId = 0;

    // Recovering older sessions
    tx::TXContext ctx;
    if (q.session_context == 0)
      ctx = tx::TransactionManager::beginTransaction();
    else {
      if (tx::TransactionManager::isRunningTransaction(q.session_context)) {
        ctx = tx::TransactionManager::getContext(q.session_context);
      } else {
        LOG4CXX_ERROR(logger, "Invalid transaction id " << q.session_context);
        _responseTask->addErrorMessage("Invalid transaction id set, aborting execution.");
      }
    }

    // JSON Parsing
    Json::Value request_data;
    Json::Reader reader;

    // Parsing the request
    if (reader.parse(q.query, request_data)) {
      _responseTask->setTxContext(ctx);

      // Modify Performance data 
      recordPerformance = request_data["performance"].asBool();
      _responseTask->setRecordPerformanceData(recordPerformance);

      // the performance attribute for this operation (at [0])
      if (recordPerformance) {
        performance_data.push_back(std::unique_ptr<hyrise::access::performance_attributes_t>(new hyrise::access::performance_attributes_t));
      }

      LOG4CXX_DEBUG(logger, request_data);

      const std::string& final_hash = hash(q.query);
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
        LOG4CXX_ERROR(logger, "Received\n:" << request_data);
        LOG4CXX_ERROR(logger, "Exception thrown during query deserialization:\n" << ex.what());
        _responseTask->addErrorMessage(std::string("RequestParseTask: ") + ex.what());
        tasks.clear();
        result = nullptr;
      }

      // Append autocommit commit operation
      if (q.autocommit) {
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
        LOG4CXX_ERROR(logger, "Json did not yield tasks");
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
      LOG4CXX_WARN(logger, "no body received!");
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
      _return.result = conn->getResponse();
      delete conn;      

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
      _return.result = conn->getResponse();
      delete conn;
    }


  }

};


class PidFile {
 public:
  PidFile() {
    std::ofstream pidf(PID_FILE);
    pidf << getpid();
  }

  ~PidFile() {
    if (remove(PID_FILE) != 0)
      perror("unlink pidfile");
  }
};

void bindToNode(int node) {
  hwloc_topology_t topology = getHWTopology();
  hwloc_cpuset_t cpuset;
  hwloc_obj_t obj;

  // The actual core
  obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_CORE, node);
  cpuset = hwloc_bitmap_dup(obj->cpuset);
  hwloc_bitmap_singlify(cpuset);

  // bind
  if (hwloc_set_cpubind(topology, cpuset, HWLOC_CPUBIND_STRICT | HWLOC_CPUBIND_NOMEMBIND | HWLOC_CPUBIND_PROCESS)) {
    char *str;
    int error = errno;
    hwloc_bitmap_asprintf(&str, obj->cpuset);
    printf("Couldn't bind to cpuset %s: %s\n", str, strerror(error));
    free(str);
    throw std::runtime_error(strerror(error));
  }

  // free duplicated cpuset
  hwloc_bitmap_free(cpuset);

  obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_MACHINE, node);
  if (hwloc_set_membind_nodeset(topology, obj->nodeset, HWLOC_MEMBIND_INTERLEAVE, HWLOC_MEMBIND_STRICT | HWLOC_MEMBIND_THREAD)) {
    char *str;
    int error = errno;
    hwloc_bitmap_asprintf(&str, obj->nodeset);
    fprintf(stderr, "Couldn't membind to nodeset  %s: %s\n", str, strerror(error));
    fprintf(stderr, "Continuing as normal, however, no guarantees\n");
    free(str);
  }
}


int main(int argc, char *argv[]) {
  size_t port = 0;
  int worker_threads = 0;
  std::string logPropertyFile;
  std::string scheduler_name;

  // Program Options
  po::options_description desc("Allowed Parameters");
  desc.add_options()("help", "Shows this help message")
  ("port,p", po::value<size_t>(&port)->default_value(DEFAULT_PORT), "Server Port")
  ("logdef,l", po::value<std::string>(&logPropertyFile)->default_value("build/log.properties"), "Log4CXX Log Properties File")
  ("scheduler,s", po::value<std::string>(&scheduler_name)->default_value("ThreadPerTaskScheduler"), "Name of the scheduler to use")
  ("threads,t", po::value<int>(&worker_threads)->default_value(getNumberOfCoresOnSystem()), "Number of worker threads for scheduler (only relevant for scheduler with fixed number of threads)");
  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch(po::error &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }


  //Bind the program to the first NUMA node for schedulers that have core bound threads
  if((scheduler_name == "CoreBoundQueuesScheduler") || (scheduler_name == "CoreBoundQueuesScheduler") ||  (scheduler_name == "WSCoreBoundQueuesScheduler") || (scheduler_name == "WSCoreBoundPriorityQueuesScheduler"))
    bindToNode(0);

  // Log File Configuration
  PropertyConfigurator::configure(logPropertyFile);

#ifndef PRODUCTION
  LOG4CXX_WARN(logger, "compiled with development settings, expect substantially lower and non-representative performance");
#endif

  SharedScheduler::getInstance().init(scheduler_name, worker_threads);

  PidFile pi;
  boost::shared_ptr<HyriseHandler> handler(new HyriseHandler());
  boost::shared_ptr<apache::thrift::TProcessor> processor(new HyriseProcessor(handler));
  boost::shared_ptr<apache::thrift::transport::TServerTransport> serverTransport(new apache::thrift::transport::TServerSocket(port));
  boost::shared_ptr<apache::thrift::transport::TTransportFactory> transportFactory(new apache::thrift::transport::TBufferedTransportFactory());
  boost::shared_ptr<apache::thrift::protocol::TProtocolFactory> protocolFactory(new apache::thrift::protocol::TBinaryProtocolFactory());

  
  LOG4CXX_INFO(logger, "Started server on port " << DEFAULT_PORT);
  apache::thrift::server::TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  LOG4CXX_INFO(logger, "Stopping Server...");

  return 0;
}
