// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SIMPLETABLESCAN_H_
#define SRC_LIB_ACCESS_SIMPLETABLESCAN_H_

#include "access/system/ParallelizablePlanOperation.h"
#include "access/expressions/pred_SimpleExpression.h"
#include "helper/serialization.h"

namespace hyrise {
namespace access {

class SimpleTableScan : public ParallelizablePlanOperation {

public:
  struct Parameters
  {
    std::string type;
    Json::Value predicates;
    std::optional<bool> materializing, ofDelta;

    SERIALIZE(type, predicates, materializing, ofDelta)
  };

public:
  SimpleTableScan();
  SimpleTableScan(const Parameters & parameters);
  virtual ~SimpleTableScan();

  void setupPlanOperation();
  void executePlanOperation();
  void executePositional();
  void executeMaterialized();
  const std::string vname();
  void setPredicate(SimpleExpression *c);

private:
  SimpleExpression *_comparator;
  bool _ofDelta;
};

}
}

#endif  // SRC_LIB_ACCESS_SIMPLETABLESCAN_H_
