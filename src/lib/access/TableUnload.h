// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TABLEUNLOAD_H_
#define SRC_LIB_ACCESS_TABLEUNLOAD_H_

#include <access/PlanOperation.h>

class TableUnload : public _PlanOperation {
 public:

  TableUnload() {
    
  }

  virtual ~TableUnload() {
    
  }

  void executePlanOperation();

  void setTableName(std::string tablename) {
    _table_name = tablename;
  }

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;
  static std::string name() {
    return "TableUnload";
  }

  const std::string vname() {
    return "TableUnload";
  }

 private:
  std::string _table_name;
  std::string _file_name;
};

#endif  // SRC_LIB_ACCESS_TABLEUNLOAD_H_

