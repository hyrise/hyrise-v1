// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdlib.h>
#include <thread>

namespace hyrise {
namespace io {

class CheckpointDaemon {
 public:
  static CheckpointDaemon& getInstance();
  void start(size_t checkpointing_interval_in_ms);
  void stop();

 private:
  CheckpointDaemon();
  ~CheckpointDaemon();

  void run();
  void doCheckpoint();

  bool _running;
  size_t _checkpointing_interval_in_ms;
  std::thread _thread;
};
}
}
