// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "RawTable.h"



namespace hyrise {

template <>
std::string to_string(const std::string& val) { return val; }
template <>
std::string to_string(std::string& val) { return val; }
template <>
std::string to_string(std::string val) { return val; }


namespace storage { namespace rawtable {

template<>
std::string RowHelper::convert(const RowHelper::byte* d, DataType t) {
  std::string result((char*) d + 2, *((uint16_t*) d));
  return result;
}

template<>
void RowHelper::set(size_t index, std::string val) {
  byte* tmp = (byte*) malloc(2 + val.size());
  memcpy(tmp+2, (byte*) val.c_str(), val.size());
  *((unsigned short*) tmp) = static_cast<uint16_t>(val.size());
  _tempData[index] = tmp;
}

}}}
