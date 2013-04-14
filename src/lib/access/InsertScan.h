// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INSERTSCAN_H_
#define SRC_LIB_ACCESS_INSERTSCAN_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

class InsertScan : public _PlanOperation {
public:
  virtual ~InsertScan();

  void executePlanOperation();
  const std::string vname();
  void setInputData(const storage::atable_ptr_t &c);

private:
  storage::atable_ptr_t _data;
};

}
}

#endif  // SRC_LIB_ACCESS_INSERTSCAN_H_
