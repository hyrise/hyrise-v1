// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MERGETABLE_H_
#define SRC_LIB_ACCESS_MERGETABLE_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class MergeTable : public PlanOperation {
public:
  virtual ~MergeTable();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
};

class MergeStore : public PlanOperation {
public:
  virtual ~MergeStore();
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
};


}
}

#endif /* SRC_LIB_ACCESS_MERGETABLE_H_ */
