#ifndef SRC_LIB_ACCESS_FORWARD_H_
#define SRC_LIB_ACCESS_FORWARD_H_

#include "access/PlanOperation.h"

namespace hyrise { namespace access {

class Forward : public _PlanOperation {
 public:
  void executePlanOperation();
};

}}

#endif 
