// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_METADATA_H_
#define SRC_LIB_ACCESS_METADATA_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class MetaData : public PlanOperation {
public:
  void executePlanOperation();
};

}
}

#endif  // SRC_LIB_ACCESS_METADATA_H_
