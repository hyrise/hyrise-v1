// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TABLEIO_H
#define SRC_LIB_ACCESS_TABLEIO_H

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class DumpTable : public PlanOperation {

  std::string _name;

public:
  virtual ~DumpTable() = default;

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

};

class LoadDumpedTable : public PlanOperation {

  std::string _name;

public:
  virtual ~LoadDumpedTable() = default;

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

};

}
}

#endif /* SRC_LIB_ACCESS_TABLEIO_H */
