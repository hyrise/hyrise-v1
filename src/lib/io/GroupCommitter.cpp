// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <io/GroupCommitter.h>

#include <helper/HwlocHelper.h>
#include <helper/Settings.h>
#include <io/logging.h>

#include <sys/time.h>
#include <iostream>

namespace hyrise {
namespace io {

GroupCommitter& GroupCommitter::getInstance() {
  static GroupCommitter g;
  return g;
};

void GroupCommitter::push(ENTRY_T entry) { _queue.push(entry); }

GroupCommitter::GroupCommitter() : _running(true) { _thread = std::thread(&GroupCommitter::run, this); }

GroupCommitter::~GroupCommitter() {
  _running = false;
  _thread.join();
}

void GroupCommitter::run() {
  // bind to numa node
  if (getNumberOfNodesOnSystem() > 1) {
    bindCurrentThreadToNumaNode(0);
  }

  struct timeval time_start, time_cur;
  unsigned long long elapsed_usec;
  gettimeofday(&time_start, NULL);

  while (_running) {

    std::this_thread::yield();

    ENTRY_T tmp;
    while (_queue.try_pop(tmp)) {
      _toBeFlushed.push_back(tmp);
    }

    gettimeofday(&time_cur, NULL);
    elapsed_usec = (time_cur.tv_sec * 1000000 + time_cur.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec);

    if (elapsed_usec > Settings::getInstance()->commit_window_ms * 1000) {
      Logger::getInstance().flush();
      respondClients();
      time_start = time_cur;
    }
  }
}

void GroupCommitter::respondClients() {
  for (ENTRY_T& entry : _toBeFlushed) {
    net::AbstractConnection* connection;
    size_t status;
    std::string response;
    std::tie(connection, status, response) = entry;
    if (connection != nullptr) {
      connection->respond(response, status);
    }
  }
  _toBeFlushed.clear();
}
}
}
