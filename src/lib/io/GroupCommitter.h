// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_GROUPCOMMITER_H
#define SRC_LIB_IO_GROUPCOMMITER_H

#include <net/AbstractConnection.h>
#include <taskscheduler/Task.h>

#include <tbb/concurrent_queue.h>

namespace hyrise {
namespace io {

class GroupCommitter {
 public:
  typedef std::tuple<net::AbstractConnection*, size_t, std::string> ENTRY_T;

  static GroupCommitter& getInstance();
  void push(ENTRY_T entry);

 private:
  GroupCommitter();
  ~GroupCommitter();

  void run();
  void respondClients();

  bool _running;
  std::thread _thread;
  tbb::concurrent_queue<ENTRY_T> _queue;
  std::vector<ENTRY_T> _toBeFlushed;
};
}
}

#endif
