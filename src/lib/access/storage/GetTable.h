// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_GETTABLE_H
#define SRC_LIB_ACCESS_GETTABLE_H

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

/// Provides the ability to get an already loaded table
/// from StorageManager for use in subsequent steps
class GetTable : public PlanOperation {
public:
  GetTable(const std::string &name);
  virtual ~GetTable();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();

private:
  const std::string _name;
};

}
}

#endif /* SRC_LIB_ACCESS_GETTABLE_H */
