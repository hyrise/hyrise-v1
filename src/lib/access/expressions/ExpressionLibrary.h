// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPRESSIONLIBRARY_H_
#define SRC_LIB_ACCESS_EXPRESSIONLIBRARY_H_

#include <map>
#include "helper/types.h"
#include <stdexcept>
#include <iostream>
#include "access/expressions/Expression.h"
#include "storage/storage_types.h"
#include "json.h"

namespace hyrise {
namespace access {

class ExpressionLibrary {
 public:
  bool add(const std::string& name, std::unique_ptr<Expression>(*function)(const Json::Value&));

  std::unique_ptr<Expression> dispatchAndParse(const Json::Value& data);

  static ExpressionLibrary& getInstance() {
    static ExpressionLibrary instance;

    return instance;
  }

  std::map<std::string, std::unique_ptr<Expression>(*)(const Json::Value&)> Expressions;

 private:
  ExpressionLibrary() {};

  ExpressionLibrary(ExpressionLibrary const&);
  void operator=(ExpressionLibrary const&);
};
}
}

#endif