// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "json_converters.h"
#include <storage/storage_types.h>

#include <stdexcept>

template <typename T>
T json_converter::convert(Json::Value v) {
  throw std::runtime_error("DataType not supported!");
}

template <>
int json_converter::convert<int>(Json::Value v) {
  return v.asInt();
}

template <>
hyrise_int_t json_converter::convert<hyrise_int_t>(Json::Value v) {
  return v.asInt64();
}


template <>
float json_converter::convert<float>(Json::Value v) {
  return v.asDouble();
}

template <>
std::string json_converter::convert<std::string>(Json::Value v) {
  return v.asString();
}
