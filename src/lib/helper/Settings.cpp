// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "Settings.h"

size_t Settings::getThreadpoolSize() const {
  return this->threadpoolSize;
}

void Settings::setThreadpoolSize(const size_t newSize) {
  this->threadpoolSize = newSize;
}

