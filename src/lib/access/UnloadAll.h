// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_UNLOADALL_H_
#define SRC_LIB_ACCESS_UNLOADALL_H_

#include <iostream>
#include <access/PlanOperation.h>

class UnloadAll : public _PlanOperation {
 public:

  UnloadAll() {
    
  }

  virtual ~UnloadAll() {
  }

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  static bool is_registered;

  static std::string name() {
    return "UnloadAll";
  }

  const std::string vname() {
    return "UnloadAll";
  }

};

#endif  // SRC_LIB_ACCESS_UNLOADALL_H_

