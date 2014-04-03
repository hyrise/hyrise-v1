// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace taskscheduler {

SharedScheduler& SharedScheduler::getInstance() {
  static SharedScheduler s;
  return s;
}
}
}  // namespace hyrise::taskscheduler
