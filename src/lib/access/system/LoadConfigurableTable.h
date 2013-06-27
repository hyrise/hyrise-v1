// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_LOADCONFIGURABLETABLE_H_
#define SRC_LIB_ACCESS_LOADCONFIGURABLETABLE_H_

#include "access/system/PlanOperation.h"
#include "storage/ColumnProperties.h"

namespace hyrise {
namespace access {

class LoadConfigurableTable : public _PlanOperation {
public:
  explicit LoadConfigurableTable(const std::string &filename, std::shared_ptr<ColumnProperties> colProperties);
  virtual ~LoadConfigurableTable();

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();

private:
  const std::string _filename;
  const std::shared_ptr<ColumnProperties> _colProperties;
};

}
}


#endif  // SRC_LIB_ACCESS_LOADCONFIGURABLETABLE_H_
