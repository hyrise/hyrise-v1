// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SettingsOperation.h"

#include "access/QueryParser.h"

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
  Settings::getInstance()->setThreadpoolSize(_threadpoolSize);
}

std::shared_ptr<_PlanOperation> SettingsOperation::parse(Json::Value &data) {
  std::shared_ptr<SettingsOperation> settingsOp = std::make_shared<SettingsOperation>();
  settingsOp->setThreadpoolSize(data["threadpoolSize"].asUInt());
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
