// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_LAYOUTTABLELOAD_H_
#define SRC_LIB_ACCESS_LAYOUTTABLELOAD_H_
#include "PlanOperation.h"

class LayoutTableLoad : public _PlanOperation {
 public:
  LayoutTableLoad();
  virtual ~LayoutTableLoad();

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;
  void setTableName(std::string tablename) {
    _table_name = tablename;
  }
  void setFileName(std::string filename) {
    _file_name = filename;
  }
  void setOverrideGroup(std::string group) {
    _override_group = group;
  }
  void setInputRow(size_t row) {
    _input_row = row;
  }
  void setUnsafe(bool unsafe) {
    _unsafe = unsafe;
  }

  static std::string name() {
    return "LayoutTableLoad";
  }
  const std::string vname() {
    return "LayoutTableLoad";
  }
 private:
  std::string _table_name;
  std::string _file_name;
  std::string _override_group;
  size_t _input_row;
  bool _unsafe;
};

#endif  // SRC_LIB_ACCESS_LAYOUTTABLELOAD_H_

