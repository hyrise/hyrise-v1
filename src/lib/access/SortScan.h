// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SORTSCAN_H_
#define SRC_LIB_ACCESS_SORTSCAN_H_

#include <access/system/PlanOperation.h>

namespace hyrise {
namespace access {

class SortScan : public PlanOperation {
public:
  virtual ~SortScan();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setSortField(const unsigned s);
  void setSortFieldName(const std::string& name);

private:
  unsigned _sort_field;
  std::string _sort_field_name;
  bool asc = true;
};

}
}

#endif  // SRC_LIB_ACCESS_SORTSCAN_H_
