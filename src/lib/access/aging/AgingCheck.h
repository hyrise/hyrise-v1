// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>

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
    std::string name;
    DataType type;
    void* data;
  };
  typedef std::vector<field_data_t> field_vector_t;
  std::map<std::string, field_vector_t> _fields;
};

} } // namespace hyrise::access

