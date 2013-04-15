// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

class LoadWithDefaultDict : public _PlanOperation {
public:
  explicit LoadWithDefaultDict(const std::string &filename, const bool forceMutable=false);
  virtual ~LoadWithDefaultDict();

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();

private:
  const std::string _filename;
  const bool _forceMutable;
};

}
}

