// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SORTSCAN_H_
#define SRC_LIB_ACCESS_SORTSCAN_H_

#include <access/PlanOperation.h>


class SortScan : public _PlanOperation {
 public:

  SortScan() {
    
  }

  virtual ~SortScan() {
  }

  void setSortField(unsigned s) {
    _sort_field = s;
  }

  void executePlanOperation();


  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;
  static std::string name() {
    return "SortScan";
  }

  const std::string vname() {
    return "SortScan";
  }

 private:

  unsigned _sort_field;
};

#endif  // SRC_LIB_ACCESS_SORTSCAN_H_

