// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SETTINGSOPERATION_H_
#define SRC_LIB_ACCESS_SETTINGSOPERATION_H_

#include <access/PlanOperation.h>

/*  This operation is used to configure global settings as long as dedicated
    units handling such decisions are not implemented.  */
class SettingsOperation : public _PlanOperation {
  size_t threadpoolSize;
 public:
  static bool is_registered;

  SettingsOperation() :
      threadpoolSize(1) {
    
  }

  virtual ~SettingsOperation() {
    
  }

  static std::string name() {
    return "SettingsOperation";
  }
  const std::string vname() {
    return "SettingsOperation";
  }

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  void executePlanOperation();

  //  Set the maximum number of parallel executable operation tasks.
  void setThreadpoolSize(const size_t newSize);
};

#endif  // SRC_LIB_ACCESS_SETTINGSOPERATION_H_

