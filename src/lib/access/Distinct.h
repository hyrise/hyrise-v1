// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCES_DISTINCT_H
#define SRC_LIB_ACCES_DISTINCT_H

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements the distinct operator for any kind of input table.
/// It has linear complexity since it scans the attribute and retrieves
/// all distinct valueIds and builds the result.
class Distinct : public PlanOperation {
public:
  virtual ~Distinct();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
};

}
}

#endif // SRC_LIB_ACCES_DISTINCT_H
