// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "json_converters.h"
#include <storage/storage_types.h>

#include <stdexcept>

template<typename T>
T json_converter::convert(const rapidjson::Value& v) {
  throw std::runtime_error("DataType not supported!");
}

template<>
int json_converter::convert<int>(const rapidjson::Value& v) {
  return v.GetInt();
}

template<>
hyrise_int_t json_converter::convert<hyrise_int_t>(const rapidjson::Value& v) {
  return v.GetInt64();
}


template<>
float json_converter::convert<float>(const rapidjson::Value& v) {
  return v.GetDouble();
}

template<>
std::string json_converter::convert<std::string>(const rapidjson::Value& v) {
  return v.GetString();
}

