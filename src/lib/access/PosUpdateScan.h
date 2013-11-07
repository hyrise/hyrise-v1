// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_POSUPDATESCAN_H_
#define SRC_LIB_ACCESS_POSUPDATESCAN_H_

#include "access/system/PlanOperation.h"
#include <unordered_map>

namespace hyrise {
namespace access {

/*
* Provides positional update capabilities
*
* The input to this plan operation is a list of positions and the updated
* value data from the JSON string.
*/
class PosUpdateScan : public PlanOperation {
public:

  virtual ~PosUpdateScan();

  void executePlanOperation();
  
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

  void setRawData(const Json::Value& d);

private:
  
  std::unordered_map<std::string, Json::Value> _raw_data;
};

}
}

#endif  // SRC_LIB_ACCESS_POSUPDATESCAN_H_
