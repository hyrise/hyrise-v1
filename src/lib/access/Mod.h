// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MOD_H_
#define SRC_LIB_ACCESS_MOD_H_

#include "access/system/ParallelizablePlanOperation.h"
#include "storage/storage_types.h"

namespace hyrise {
namespace access {

/// This class implements the mod function as a hyrise operation
/// You can set the divisor and the field that should be affected
/// This operation will create a new column containing the result values
/// Usage:
/// ........
///   "mod" :{
///       "type" : "Mod",
///       "fields" : ["C_ID"],
///       "vtype" : 0,
///       "divisor" : 10,
///       "as" : "MOD_FIELD"
///   },
/// ........
/// "fields" contains the field that acts as divident (should only have one element)
/// "vtype" should define the value type of the operation (int = 0, float = 1)
/// "divisor" is the number, that devides the given field
/// "as" contains the name of the new column

class Mod : public ParallelizablePlanOperation {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

  void setDivisor(const float& divisor);
  void setVType(const int& vtype);
  void setColName(const std::string& colName);

  float getDivisor() const;
  int getVType() const;
  std::string getColName() const;

 private:
  float _divisor = 1.f;
  int _vtype = IntegerType;
  std::string _colName;
};
}
}
#endif  // SRC_LIB_ACCESS_MOD_H_
