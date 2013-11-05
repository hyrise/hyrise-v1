// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SETTABLE_H
#define SRC_LIB_ACCESS_SETTABLE_H

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

/// Provides the ability to put a table into the StorageManager
class SetTable : public PlanOperation
{
public:
  SetTable(const std::string& name);
  virtual ~SetTable();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();

private:
  const std::string _name;
};

}
}

#endif /* SRC_LIB_ACCESS_SETTABLE_H */
