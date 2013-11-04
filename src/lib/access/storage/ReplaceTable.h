// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_REPLACETABLE_H
#define SRC_LIB_ACCESS_REPLACETABLE_H

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

/// Provides the ability to put a table into the StorageManager
class ReplaceTable : public PlanOperation {
public:
  ReplaceTable(const std::string &name);
  virtual ~ReplaceTable();

  void executePlanOperation();
  const std::string vname();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

private:
  const std::string _name;
};

}
}

#endif /* SRC_LIB_ACCESS_REPLACETABLE_H */
