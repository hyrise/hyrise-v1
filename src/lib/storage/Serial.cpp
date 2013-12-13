#include "Serial.h"

namespace hyrise {
namespace storage {


Serial::Serial() {
  reset();
}

void Serial::reset() {
  _serial = 1ul;
}

Serial::serial_t Serial::next() {
  return _serial++;
}

} } // namespace hyrise::storage
