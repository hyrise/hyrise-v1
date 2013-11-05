// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INSERTSCAN_H_
#define SRC_LIB_ACCESS_INSERTSCAN_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class InsertScan : public PlanOperation {
public:
  virtual ~InsertScan();

  void executePlanOperation();

  void setInputData(const storage::atable_ptr_t &c);

  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

private:
  
	storage::atable_ptr_t buildFromJson();

  storage::atable_ptr_t _data;

  std::vector<std::vector<Json::Value>> _raw_data;
};

}
}

#endif  // SRC_LIB_ACCESS_INSERTSCAN_H_
