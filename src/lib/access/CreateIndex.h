// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_CREATEINDEX_H_
#define SRC_LIB_ACCESS_CREATEINDEX_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// column needs to be passed via "fields" json field
class CreateIndex : public _PlanOperation {
public:
  /**
   * parse JSON fields
   * set index name in field "table_name"
   * set column in field "fields"
   */
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
  void setTableName(const std::string &t);

protected:
  virtual void executePlanOperation();

private:
  std::string _table_name;
};

}
}

#endif  // SRC_LIB_ACCESS_CREATEINDEX_H_
