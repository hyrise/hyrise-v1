// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <stdexcept>
#include <boost/algorithm/string.hpp>

#include <hwloc.h>
#include <signal.h>

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

#include <boost/program_options.hpp>

#include "helper/HwlocHelper.h"
#include "helper/Settings.h"
#include "net/AsyncConnection.h"
#include "io/StorageManager.h"
#include "access/CheckpointDaemon.h"
#include "taskscheduler/SharedScheduler.h"

namespace po = boost::program_options;
using namespace hyrise;
using namespace log4cxx;
using namespace log4cxx::helpers;

namespace {

const char* PID_FILE = "./hyrise_server.pid";
const char* PORT_FILE = "./hyrise_server.port";
const size_t DEFAULT_PORT = 5000;
// default maximum task size. 0 is disabled.
const size_t DEFAULT_MTS = 0;


LoggerPtr logger(Logger::getLogger("hyrise"));
}

class InfoFile {
 public:
  template <typename T>
  InfoFile(std::string filename, T value)
      : _filename(filename) {
    std::ofstream of(filename);
    of << value;
  }

  ~InfoFile() {
    if (remove(_filename.c_str()) != 0) {
      perror(("unlinking info file" + _filename).c_str());
    }
  }

 private:
  std::string _filename;
};

int main(int argc, char* argv[]) {
  size_t port = 0;
  int worker_threads = 0;
  std::string logPropertyFile;
  std::string scheduler_name;
  size_t maxTaskSize;
  std::string numa_nodes_str;
  size_t checkpoint_interval = 0;
  bool recover = 0;
  bool recoverAndExit = 0;
  size_t commit_window_ms = 0;

  // Program Options
  po::options_description desc("Allowed Parameters");
  desc.add_options()("help", "Shows this help message")(
      "port,p", po::value<size_t>(&port)->default_value(DEFAULT_PORT), "Server Port")(
      "logdef,l",
      po::value<std::string>(&logPropertyFile)->default_value("build/log.properties"),
      "Log4CXX Log Properties File")(
      "maxTaskSize,m",
      po::value<size_t>(&maxTaskSize)->default_value(DEFAULT_MTS),
      "Maximum task size used in dynamic parallelization scheduler. Use 0 for unbounded task run time.")(
      "scheduler,s",
      po::value<std::string>(&scheduler_name)->default_value("CentralScheduler"),
      "Name of the scheduler to use")
      // set default number of worker threads to #cores-1, as main thread with event loop is bound to core 0
      ("threads,t",
       po::value<int>(&worker_threads)->default_value(getNumberOfCoresPerNumaNode() - NUM_RESERVED_CORES),
       "Number of worker threads for scheduler (only relevant for scheduler with fixed number of threads)")(
          "recover,r", po::value<bool>(&recover)->zero_tokens(), "Recover tables on load")(
          "recoverAndExit,x",
          po::value<bool>(&recoverAndExit)->zero_tokens(),
          "Recover tables on load and exit (for benchmarking purposes)")(
          "nodes",
          po::value<std::string>(&numa_nodes_str)->default_value(""),
          "comma-separated list of NUMA-nodes to use (e.g., 0,2) - defaults to all - CURRENTLY UNUSED")
#ifdef PERSISTENCY_BUFFEREDLOGGER
      ("checkpointInterval,c",
       po::value<size_t>(&checkpoint_interval)->default_value(0),
       "Interval for checkpointing in ms")(
          "commitWindow", po::value<size_t>(&commit_window_ms)->default_value(50), "Commit window in ms")
#endif
      ;
  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch (po::error& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  // separate numa_nodes
  std::vector<size_t> numa_nodes;
  if (!numa_nodes_str.empty()) {
    std::vector<std::string> numa_nodes_str_v;
    boost::split(numa_nodes_str_v, numa_nodes_str, boost::is_any_of(","));
    for (auto&& n : numa_nodes_str_v)
      numa_nodes.push_back(atoi(n.c_str()));
  } else {
    uint number_of_nodes = getNumberOfNodesOnSystem();
    for (uint i = 0; i < number_of_nodes; ++i)
      numa_nodes.push_back(i);
  }

  // get available cores
  std::vector<size_t> numa_cores;
  for (auto n : numa_nodes) {
    for (auto c : getCoresForNode(getHWTopology(), n))
      numa_cores.push_back(c);
  }


  // Bind the program to the first NUMA node for schedulers that have core bound threads
  // set number of threads to core-count -1
  if ((scheduler_name == "CoreBoundQueuesScheduler") || (scheduler_name == "WSCoreBoundQueuesScheduler") ||
      (scheduler_name == "WSCoreBoundPriorityQueuesScheduler") ||
      (scheduler_name == "CoreBoundPriorityQueuesScheduler")) {
    bindCurrentThreadToCore(0);
    if (worker_threads == -1)
      worker_threads = getNumberOfCoresOnSystem() - 1;
  }
  // if no core bound scheduler, set the number of threads to core-count
  else {
    if (worker_threads == -1)
      worker_threads = getNumberOfCoresOnSystem();
  }

  Settings::getInstance()->worker_threads = worker_threads;
  Settings::getInstance()->port = port;
  Settings::getInstance()->scheduler_name = scheduler_name;
  Settings::getInstance()->checkpoint_interval = checkpoint_interval;
  Settings::getInstance()->numa_nodes = numa_nodes;
  Settings::getInstance()->numa_cores = numa_cores;
  Settings::getInstance()->commit_window_ms = commit_window_ms;
  Settings::getInstance()->printInfo();


  // recovery?
  if (recover || recoverAndExit) {
    struct timeval start = {0, 0}, end = {0, 0};
    std::cout << "Recovering tables..." << std::endl;
    gettimeofday(&start, nullptr);
    io::StorageManager::getInstance()->recoverTables();
    gettimeofday(&end, nullptr);
    auto recoveryTime = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    std::cout << "Done. Recovery time was " << recoveryTime << std::endl;
    if (recoverAndExit) {
      return 0;
    }
  }

  // start checkpointing daemon?
  if (checkpoint_interval > 0) {
    io::CheckpointDaemon::getInstance().start(checkpoint_interval);
  }


  // Log File Configuration
  PropertyConfigurator::configure(logPropertyFile);

#ifndef PRODUCTION
  LOG4CXX_WARN(logger,
               "compiled with development settings, expect substantially lower and non-representative performance");
#endif

  taskscheduler::SharedScheduler::getInstance().init(scheduler_name, worker_threads, maxTaskSize);

  // Main Server Loop
  struct ev_loop* loop = ev_default_loop(0);
  ebb_server server;
  // Initialize server based on libev event loop
  ebb_server_init(&server, loop);


  // Define handler for ebb
  server.new_connection = net::new_connection;

  if (ebb_server_listen_on_port(&server, port) == -1) {
    std::cout << "Failed to start server" << std::endl;
    return EXIT_FAILURE;
  }

  InfoFile port_file(PORT_FILE, server.port);
  InfoFile pid_file(PID_FILE, getpid());

  std::cout << "Started server on port " << server.port << std::endl;
  ev_loop(loop, 0);
  LOG4CXX_INFO(logger, "Stopping Server...");
  ev_default_destroy();
  return EXIT_SUCCESS;
}
