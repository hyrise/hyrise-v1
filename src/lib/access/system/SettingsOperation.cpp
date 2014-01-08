// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/SettingsOperation.h"

#include "access/system/QueryParser.h"

#include "helper/Settings.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SettingsOperation>("SettingsOperation");
}

SettingsOperation::SettingsOperation() : _threadpoolSize(1) {
}

SettingsOperation::~SettingsOperation() {
}

void SettingsOperation::executePlanOperation() {
  
  if (_data.isMember("threadpoolSize"))
    Settings::getInstance()->setThreadpoolSize(_data["threadpoolSize"].asUInt());

  if (_data.isMember("profilePath"))
    Settings::getInstance()->setProfilePath(_data["profilePath"].asString());

}

std::shared_ptr<PlanOperation> SettingsOperation::parse(const Json::Value &data) {
  std::shared_ptr<SettingsOperation> settingsOp = std::make_shared<SettingsOperation>();
  settingsOp->_data = data;
  return settingsOp;
}

const std::string SettingsOperation::vname() {
  return "SettingsOperation";
}

void SettingsOperation::setThreadpoolSize(const size_t newSize) {
  _threadpoolSize = newSize;
}

}
}
