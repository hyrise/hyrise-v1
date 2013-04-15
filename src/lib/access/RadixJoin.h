// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_RADIXJOIN_H_
#define SRC_LIB_ACCESS_RADIXJOIN_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

class RadixJoin : public _PlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
  void setBits1(const uint32_t b);
  void setBits2(const uint32_t b);
  uint32_t bits1() const;
  uint32_t bits2() const;

private:
  uint32_t _bits1;
  uint32_t _bits2;
};

}
}

#endif  // SRC_LIB_ACCESS_RADIXJOIN_H_
