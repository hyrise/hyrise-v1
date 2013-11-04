// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCES_MULTIPLYREFFIELD_H
#define SRC_LIB_ACCES_MULTIPLYREFFIELD_H

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements a horizontal multiply operation that allows to specify
/// a reference field and a value field so that for all rows of the second
/// input table are multiplied by the value field
///
/// The input to the plan operation are two tables: the first input
/// specifies the source table and the second table the table to multiply.
/// The first input field defines the reference field while the second input
/// field defines the value field to use as a mulitplier. The format of the
/// target field depends on the value field.
class MultiplyRefField : public PlanOperation {
 public:
  void executePlanOperation();
  /// Example JSON for the plan operation, fields defines the reference and value
  /// the value field
  /// "multiply" : {
  ///   "type" : "MultiplyRefField",
  ///   "fields" : ["ref", "value"],
  /// }
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();

 private:
  template<typename T, DataType D>
  void executeMultiply();
};


}
}

#endif // SRC_LIB_ACCES_MULTIPLYREFFIELD_H
