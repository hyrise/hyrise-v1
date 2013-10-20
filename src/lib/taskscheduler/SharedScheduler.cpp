#include "taskscheduler/SharedScheduler.h"

SharedScheduler& SharedScheduler::getInstance() {
  static SharedScheduler s;
  return s;
}
