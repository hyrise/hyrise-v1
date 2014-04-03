// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <access/CheckpointDaemon.h>
#include <helper/HwlocHelper.h>
#include <access/Checkpoint.h>

#include <sys/time.h>
#include <iostream>
#include <chrono>

namespace hyrise {
namespace io {

CheckpointDaemon& CheckpointDaemon::getInstance() {
  static CheckpointDaemon g;
  return g;
};

CheckpointDaemon::CheckpointDaemon() : _running(false) {}

CheckpointDaemon::~CheckpointDaemon() {
  _running = false;
  _thread.join();
}

void CheckpointDaemon::stop() { _running = false; }

void CheckpointDaemon::start(size_t checkpointing_interval_in_ms) {
  _running = true;
  _checkpointing_interval_in_ms = checkpointing_interval_in_ms;
  _thread = std::thread(&CheckpointDaemon::run, this);
}

void CheckpointDaemon::run() {

  std::cout << "CheckpointDaemon started..." << std::endl;

  // bind to numa node
  bindCurrentThreadToNumaNode(0);

  struct timeval time_start, time_cur;
  unsigned long long elapsed_usec;
  gettimeofday(&time_start, NULL);
  std::chrono::milliseconds check_interval(500);

  while (_running) {

    std::this_thread::sleep_for(check_interval);

    gettimeofday(&time_cur, NULL);
    elapsed_usec = (time_cur.tv_sec * 1000000 + time_cur.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec);

    if (elapsed_usec > _checkpointing_interval_in_ms * 1000) {
      doCheckpoint();
      time_start = time_cur;
    }
  }
}

void CheckpointDaemon::doCheckpoint() {
  std::cout << "Checkpoint started..." << std::endl;
  access::Checkpoint cp;
  cp.setWithMain(false);
  cp.execute();
  std::cout << "Checkpoint ended..." << std::endl;
}
}
}
