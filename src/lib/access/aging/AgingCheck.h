// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>

namespace hyrise {
namespace access {

struct param_data_t {
  std::string table;
  std::string field;
  DataType type;
  void* data;
};

class AgingCheck : public PlanOperation {

public:
  AgingCheck(const std::string& query);
  ~AgingCheck();

  void executePlanOperation();

  void parameter(const std::vector<param_data_t>& parameter);

  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
private:
  const std::string _queryName;
  std::vector<param_data_t> _paramList;
};

} } // namespace hyrise::access

