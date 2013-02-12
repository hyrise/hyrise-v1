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

#include <io.h>

#include <net.h>
#include "taskscheduler/SharedScheduler.h"
#include "helper/HwlocHelper.h"

namespace po = boost::program_options;
using namespace hyrise;
using namespace log4cxx;
using namespace log4cxx::helpers;

const char *PID_FILE = "./hyrise_server.pid";
const size_t DEFAULT_PORT = 5000;

// Global EBB Server instance
static ebb_server server;

LoggerPtr logger(Logger::getLogger("hyrise"));

void writePidFile() {
  std::ofstream pidf(PID_FILE);
  pidf << getpid();
}

void removePidFile() {
  if (remove(PID_FILE) != 0)
    perror("unlink");
}

void shutdown(int sig) {
  std::cout << "Graceful stop" << std::endl;
  removePidFile();
  StorageManager::getInstance()->removeAll();
  sleep(4);
  ebb_server_unlisten(&server);
}

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
    printf("Couldn't bind memory to cpuset %s: %s\n", str, strerror(error));
    free(str);
    throw std::runtime_error(strerror(error));
  }
}


#include <access/RequestParseTask.h>

int main(int argc, char *argv[]) {

  //hyrise::access::RequestParseTask::registerURL();

  // Bind the program to the first NUMA node
  bindToNode(0);

  size_t port = 0;
  std::string logPropertyFile;
  std::string scheduler_name;

  // Program Options
  po::options_description desc("Allowed Parameters");
  desc.add_options()("help", "Shows this help message")
  ("port,p", po::value<size_t>(&port)->default_value(DEFAULT_PORT), "Server Port")
  ("logdef,l", po::value<std::string>(&logPropertyFile)->default_value("build/log.properties"), "Log4CXX Log Properties File")
  ("scheduler,s", po::value<std::string>(&scheduler_name)->default_value("SimpleTaskScheduler"), "Name of the scheduler to use");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  // Log File Configuration
  PropertyConfigurator::configure(logPropertyFile);

  // System Setup
  writePidFile();

  SharedScheduler::getInstance().init(scheduler_name);
  AbstractTaskScheduler *scheduler = SharedScheduler::getInstance().getScheduler();
  if (scheduler != NULL) {
    scheduler->resize(getNumberOfCoresOnSystem());
  }

  signal(SIGINT, &shutdown);
  // MainS erver Loop
  struct ev_loop *loop = ev_default_loop(0);

  // Initialize server based on libev event loop
  ebb_server_init(&server, loop);

  // Define handler for ebb
  server.new_connection = net::new_connection;

  // Start listening
  if (port <= 0 || port >= 65536)
    port = DEFAULT_PORT;
  ebb_server_listen_on_port(&server, port);
  LOG4CXX_INFO(logger, "Started server on port " << port);
  ev_loop(loop, 0);
  LOG4CXX_INFO(logger, "Stopping Server...");
  removePidFile();

  return 0;
}
