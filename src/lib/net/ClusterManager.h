// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <thread>

namespace hyrise {
namespace net {

class ClusterManager {
 public:

  static ClusterManager& getInstance();

 private:
  ClusterManager();
  ClusterManager(ClusterManager const&);
  void operator=(ClusterManager const&);

  void run();
  void checkForHeartbeat();
  void notifyDispatcher();

  std::thread _thread;
  bool _shouldRun = true;

  const size_t INTERVAL_ms_SECONDS = 250;
  // const size_t INTERVAL_uSECONDS = 1;
  const int64_t THRESHOLD_MILLISECONDS = 1000;
};
}
}