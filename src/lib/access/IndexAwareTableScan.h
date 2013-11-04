// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INDEXAWARETABLESCAN_SCAN
#define SRC_LIB_ACCESS_INDEXAWARETABLESCAN_SCAN

#include "access/system/PlanOperation.h"
#include "access/IndexScan.h"
#include "access/TableScan.h"
#include "access/expressions/pred_EqualsExpression.h"

namespace hyrise {
namespace access {

/// Scan an existing index for the result. Currently only EQ predicates
/// allowed for the index.
class IndexAwareTableScan : public PlanOperation {
public:
  explicit IndexAwareTableScan();
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(Json::Value &data);
  const std::string vname();
  void setIndexName(const std::string &name);
  void setValue(const hyrise_int_t value);

private:
  hyrise_int_t _value;
  std::string _index_name;
  // IndexScan _is;
  // TableScan _ts;
};

}
}

#endif // SRC_LIB_ACCESS_INDEXAWARETABLESCAN_SCAN
