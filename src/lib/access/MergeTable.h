// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MERGETABLE_H_
#define SRC_LIB_ACCESS_MERGETABLE_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

class MergeTable : public _PlanOperation {
public:
  virtual ~MergeTable();

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value& data);
  const std::string vname();
};

}
}

#endif /* SRC_LIB_ACCESS_MERGETABLE_H_ */
