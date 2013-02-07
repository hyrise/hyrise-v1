#include "Settings.h"

size_t Settings::getThreadpoolSize() const {
  return this->threadpoolSize;
}

void Settings::setThreadpoolSize(const size_t newSize) {
  this->threadpoolSize = newSize;
}

