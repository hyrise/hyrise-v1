#ifndef SRC_LIB_ACCESS_TX_ROLLBACK_H_
#define SRC_LIB_ACCESS_TX_ROLLBACK_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class Rollback : public PlanOperation {
  void executePlanOperation();
};

}
}

#endif
