#include "taskscheduler/SharedScheduler.h"

namespace hyrise {
namespace taskscheduler {

SharedScheduler& SharedScheduler::getInstance() {
  static SharedScheduler s;
  return s;
}

} } // namespace hyrise::taskscheduler

