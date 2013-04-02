// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SETTINGSOPERATION_H_
#define SRC_LIB_ACCESS_SETTINGSOPERATION_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// This operation is used to configure global settings as long as dedicated
/// units handling such decisions are not implemented.
class SettingsOperation : public _PlanOperation {
public:
  SettingsOperation();
  virtual ~SettingsOperation();

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
  void setThreadpoolSize(const size_t newSize);

private:
  size_t _threadpoolSize;
};

}
}

#endif  // SRC_LIB_ACCESS_SETTINGSOPERATION_H_
