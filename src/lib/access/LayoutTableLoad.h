// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_LAYOUTTABLELOAD_H_
#define SRC_LIB_ACCESS_LAYOUTTABLELOAD_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class LayoutTableLoad : public PlanOperation {
public:
  virtual ~LayoutTableLoad();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setTableName(const std::string &tablename);
  void setFileName(const std::string &filename);
  void setOverrideGroup(const std::string &group);
  void setInputRow(const size_t row);
  void setUnsafe(const bool unsafe);

private:
  std::string _table_name;
  std::string _file_name;
  std::string _override_group;
  size_t _input_row;
  bool _unsafe;
};

}
}

#endif  // SRC_LIB_ACCESS_LAYOUTTABLELOAD_H_
