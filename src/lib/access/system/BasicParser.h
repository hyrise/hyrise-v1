// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_BASICPARSER_H_
#define SRC_LIB_ACCESS_BASICPARSER_H_

#include <stdexcept>
#include <memory>
#include <rapidjson/rapidjson.h>

class BasicParsingException : public std::runtime_error {
 public:

  explicit BasicParsingException(const std::string &what): std::runtime_error(what)
  {}
};

template<typename T>
struct BasicParser {
  static std::shared_ptr<T> parse(const rapidjson::Value &data) {
    std::shared_ptr<T> ps = std::make_shared<T>();

    // For all fields add
    const auto& json_fields = data["fields"];

    // limit
    if (data.HasMember("limit"))
      ps->setLimit(data["limit"].GetUint());

    for (unsigned i = 0; i < json_fields.Size(); ++i) {
      if (json_fields[i].IsNumber()) {
        ps->addField(json_fields[i].GetUint());
      } else if (json_fields[i].IsString()) {
        ps->addField(json_fields[i].GetString());
      } else {
        throw BasicParsingException("Could not parse item from 'fields'");
      }
    }

    return ps;
  }
};

#endif  // SRC_LIB_ACCESS_BASICPARSER_H_
