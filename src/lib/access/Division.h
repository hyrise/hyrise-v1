// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_DIVISION_H_
#define SRC_LIB_ACCESS_DIVISION_H_

#include "access/system/ParallelizablePlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements division of columns as a hyrise operation
/// You can set the column to divide/be devided in "fields"
/// This operation will create a new column containing the result values
/// Usage:
/// ........
///   "div" :{
///       "type" : "Division",
///       "fields" : ["C_ID", "C_D_ID"],
///       "vtype" : 0,
///       "as" : "DIVISION_FIELD"
///   },
/// ........
/// "fields" contains the fields that will be divided (field1 / field2)
/// "vtype" should define the value type of the result column
/// "as" contains the name of the new column

class Division : public ParallelizablePlanOperation {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

  void setVType(const int& vtype);
  void setColName(const std::string& colName);

  int getVType() const;
  std::string getColName() const;

 private:
  int _vtype;
  std::string _colName;
};
}
}
#endif  // SRC_LIB_ACCESS_DIVISION_H_
