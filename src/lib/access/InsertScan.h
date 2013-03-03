// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INSERTSCAN_H_
#define SRC_LIB_ACCESS_INSERTSCAN_H_

#include "access/PlanOperation.h"
#include "helper/types.h"

namespace hyrise {
namespace access {

class InsertScan : public _PlanOperation {
public:
  InsertScan();
  virtual ~InsertScan();

  void setInputData(const storage::atable_ptr_t c);
  virtual void executePlanOperation();
  const std::string vname();

private:
  storage::atable_ptr_t _data;
};

}
}

#endif  // SRC_LIB_ACCESS_INSERTSCAN_H_
