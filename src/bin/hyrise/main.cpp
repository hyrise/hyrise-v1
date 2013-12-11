// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <hwloc.h>
#include <signal.h>

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

#include <boost/program_options.hpp>

#include "helper/HwlocHelper.h"
#include "net/AsyncConnection.h"
#include "io/StorageManager.h"
#include "taskscheduler/SharedScheduler.h"

namespace po = boost::program_options;
using namespace hyrise;
using namespace log4cxx;
using namespace log4cxx::helpers;

namespace  {

const char *PID_FILE = "./hyrise_server.pid";
const char *PORT_FILE = "./hyrise_server.port";
const size_t DEFAULT_PORT = 5000;
// default maximum task size. 0 is disabled.
const size_t DEFAULT_MTS = 0;


LoggerPtr logger(Logger::getLogger("hyrise"));

}

/// To prevent multiple hyrise instances from using the same port
/// we initialize
class PortResource {
 public:
  PortResource(size_t start, size_t end, ebb_server& s) : _current(0) {
    assert((start < end) && "start must be smaller than end");
    for (size_t current = start; current < end; ++current) {
      if (ebb_server_listen_on_port(&s, current) != -1) {
          _current = current;
          break;
      } else {
        std::cout << "Port " << current << " already in use, retrying" << std::endl;
      }
    }
    if (_current == 0) {
      std::cout << "no port available in range [" + std::to_string(start) + ", " + std::to_string(end) + "]" <<std::endl;
      std::exit(1);
    }
    std::ofstream port_file(PORT_FILE);
    port_file << _current;
  }

  ~PortResource() {
    if (remove(PORT_FILE) != 0)
      perror("unlink portfile");
  }

  size_t getPort() {
    return _current;
  }

 private:
  size_t _current;
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

  // assuming single machine system
  obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_MACHINE, 0);
  // set membind policy interleave for this thread
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
  size_t maxTaskSize;

  // Program Options
  po::options_description desc("Allowed Parameters");
  desc.add_options()("help", "Shows this help message")
  ("port,p", po::value<size_t>(&port)->default_value(DEFAULT_PORT), "Server Port")
  ("logdef,l", po::value<std::string>(&logPropertyFile)->default_value("build/log.properties"), "Log4CXX Log Properties File")
  ("maxTaskSize,m", po::value<size_t>(&maxTaskSize)->default_value(DEFAULT_MTS), "Maximum task size used in dynamic parallelization scheduler. Use 0 for unbounded task run time.")
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

  taskscheduler::SharedScheduler::getInstance().init(scheduler_name, worker_threads, maxTaskSize);

  // Main Server Loop
  struct ev_loop *loop = ev_default_loop(0);
  ebb_server server;
  // Initialize server based on libev event loop
  ebb_server_init(&server, loop);


  // Define handler for ebb
  server.new_connection = net::new_connection;

  PidFile pi;
  PortResource pa(port, port+100, server);

  LOG4CXX_INFO(logger, "Started server on port " << pa.getPort());
  ev_loop(loop, 0);
  LOG4CXX_INFO(logger, "Stopping Server...");
  ev_default_destroy ();
  return 0;
}
