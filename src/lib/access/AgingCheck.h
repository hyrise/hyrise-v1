// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class AgingCheck : public PlanOperation {
public:
  ~AgingCheck();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

private:
  std::string _queryName;
  struct field_data_t {
    std::string table;
    std::string name;
    DataType type;
    void* data;
  };
  std::vector<field_data_t> _fields;
};

} } // namespace hyrise::access

