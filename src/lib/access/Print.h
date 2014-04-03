// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PRINT_H_
#define SRC_LIB_ACCESS_PRINT_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class Print : public PlanOperation {
 public:
  Print();
  void executePlanOperation();
  void setLimit(size_t limit);
  void setComment(std::string comment);
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

 private:
  size_t _limit;
  std::string _comment;
};
}
}

#endif  // SRC_LIB_ACCESS_PRINT_H_
