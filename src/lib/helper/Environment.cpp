// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "Environment.h"
#include <stdlib.h>

std::string getEnv(std::string var, std::string _default) {
  const char *value = getenv(var.c_str());
  if (value == nullptr) return _default;
  return value;
}

