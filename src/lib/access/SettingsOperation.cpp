// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "SettingsOperation.h"
#include <access/QueryParser.h>
#include <helper/Settings.h>

bool SettingsOperation::is_registered = QueryParser::registerPlanOperation<SettingsOperation>();

bool aaa_cmp(unsigned char *p, unsigned int i) {
  return *((unsigned int *) p) == i;
}


std::shared_ptr<_PlanOperation> SettingsOperation::parse(Json::Value &data) {
  std::shared_ptr<SettingsOperation> settingsOp = std::make_shared<SettingsOperation>();
  settingsOp->threadpoolSize = data["threadpoolSize"].asUInt();
  return settingsOp;
}

void SettingsOperation::executePlanOperation() {
  Settings::getInstance()->setThreadpoolSize(this->threadpoolSize);
}

void SettingsOperation::setThreadpoolSize(const size_t newSize) {
  this->threadpoolSize = newSize;
}

