// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>

namespace hyrise {
namespace access {

class AgingCheck : public PlanOperation {
  struct param_data_t {
    std::string table;
    std::string field;
    DataType type;
    void* data;
  };

public:
  ~AgingCheck();

  void executePlanOperation();
  std::pair<std::string, bool> handleOneTable(std::vector<param_data_t>& paramList);

  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

private:
  std::string _queryName;
  std::vector<param_data_t> _paramList;
};

} } // namespace hyrise::access

