#include "Serial.h"

Serial::Serial() {
  reset();
}

void Serial::reset() {
  _serial = 1ul;
}

Serial::serial_t Serial::next() {
  return _serial++;
}