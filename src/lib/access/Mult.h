// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MULT_H_
#define SRC_LIB_ACCESS_MULT_H_

#include "access/system/ParallelizablePlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements multiplication of columns as a hyrise operation
/// You can set the columns to multiply in "fields" and an additional factor in "factor"
/// This operation will create a new column containing the result values
/// Usage:
/// ........
///   "mul" :{
///       "type" : "Mult",
///       "fields" : ["C_ID", "C_ID"],
///       "factor" : 10.4,
///       "vtype" : 0,
///       "as" : "MULT_FIELD"
///   },
/// ........
/// "fields" contains the fields that are multiplied
/// "factor" is the optional factor to multiply with
/// "vtype" should define the value type of the result column
/// "as" contains the name of the new column

class Mult : public ParallelizablePlanOperation {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

  void setVType(const int& vtype);
  void setFactor(const float& factor);
  void setColName(const std::string& colName);

  int getVType() const;
  float getFactor() const;
  std::string getColName() const;

 private:
  int _vtype;
  float _factor;
  std::string _colName;
};
}
}
#endif  // SRC_LIB_ACCESS_MULT_H_
