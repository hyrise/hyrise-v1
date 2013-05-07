// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "access/PlanOperation.h"
#include "storage/ColumnProperties.h"

namespace hyrise {
namespace access {

class LoadConfigurableTable : public _PlanOperation {
public:
  explicit LoadConfigurableTable(const std::string &filename, const PColumnProperties);
  virtual ~LoadConfigurableTable();

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();

private:
  const std::string _filename;
  const PColumnProperties _colProperties;
};

}
}

