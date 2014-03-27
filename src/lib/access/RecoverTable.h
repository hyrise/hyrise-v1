// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/system/PlanOperation.h>

namespace hyrise {
namespace access {

class RecoverTable : public PlanOperation {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
  void setTableName(const std::string& name);
  void setPath(const std::string& path);
  void setNumberThreads(const size_t thread_count);

 private:
  std::string _tableName;
  std::string _path;
  size_t _threadCount;
};
}
}
