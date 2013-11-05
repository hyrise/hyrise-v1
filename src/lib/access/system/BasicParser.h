// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_BASICPARSER_H_
#define SRC_LIB_ACCESS_BASICPARSER_H_

#include <stdexcept>
#include <memory>
#include "json.h"

class BasicParsingException : public std::runtime_error {
 public:

  explicit BasicParsingException(const std::string &what): std::runtime_error(what)
  {}
};

template<typename T>
struct BasicParser {
  static std::shared_ptr<T> parse(const Json::Value &data) {
    std::shared_ptr<T> ps = std::make_shared<T>();

    // For all fields add
    const Json::Value json_fields = data["fields"];

    // limit
    if (data.isMember("limit"))
      ps->setLimit(data["limit"].asUInt());

    for (unsigned i = 0; i < json_fields.size(); ++i) {
      if (json_fields[i].isNumeric()) {
        ps->addField(json_fields[i].asUInt());
      } else if (json_fields[i].isString()) {
        ps->addField(json_fields[i].asString());
      } else {
        throw BasicParsingException("Could not parse item from 'fields'");
      }
    }

    return ps;
  }
};

#endif  // SRC_LIB_ACCESS_BASICPARSER_H_
