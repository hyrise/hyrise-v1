#ifndef SRC_LIB_ACCESS_JSON_CONVERTERS_H_
#define SRC_LIB_ACCESS_JSON_CONVERTERS_H_

#include <json.h>

struct json_converter {
  template<typename T>
  static T convert(Json::Value v);
};

#endif  // SRC_LIB_ACCESS_JSON_CONVERTERS_H_
