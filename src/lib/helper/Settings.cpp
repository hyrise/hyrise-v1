// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "Settings.h"
#include "Environment.h"

Settings::Settings() : threadpoolSize(1) {

  // Initiate the class based on Enviroment Variables
  setDBPath(getEnv("HYRISE_DB_PATH", ""));
  setScriptPath(getEnv("HYRISE_SCRIPT_PATH", ""));
  setProfilePath(getEnv("HYRISE_PROFILE_PATH","."));

}

size_t Settings::getThreadpoolSize() const {
  return this->threadpoolSize;
}

void Settings::setThreadpoolSize(const size_t newSize) {
  this->threadpoolSize = newSize;
}

