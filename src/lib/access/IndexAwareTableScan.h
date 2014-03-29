// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INDEXAWARETABLESCAN_SCAN
#define SRC_LIB_ACCESS_INDEXAWARETABLESCAN_SCAN

#include "access/system/PlanOperation.h"
#include "access/IndexScan.h"
#include "access/TableScan.h"
#include "access/expressions/pred_EqualsExpression.h"

namespace hyrise {
namespace access {

struct GroupkeyIndexFunctor;

/// Scan an existing index for the result. Currently only EQ predicates
/// allowed for the index.
class IndexAwareTableScan : public PlanOperation {
 public:
  explicit IndexAwareTableScan();
  void executePlanOperation();
  ~IndexAwareTableScan();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  const std::string vname();
  void setPredicate(AbstractExpression* c);
  void setTableName(const std::string& name);
  void setPerformValidation(bool validate);
  void setFunctorsFromInfo(const std::string& indexname_main,
                           const std::string& indexname_delta,
                           std::string fieldname,
                           Json::Value value);

 private:
  void _setPredicateFromJson(AbstractExpression* c);
  void _getIndexResults(std::shared_ptr<const storage::Store> t_store,
                        pos_list_t*& result,
                        std::vector<GroupkeyIndexFunctor>& functors);
  void _consolidateFunctors(std::shared_ptr<const storage::Store> t_store, std::vector<GroupkeyIndexFunctor>& functors);

  SimpleExpression* _predicate;
  std::vector<GroupkeyIndexFunctor> _idx_functors_main;
  std::vector<GroupkeyIndexFunctor> _idx_functors_delta;
  std::string _tableName;
  bool _performValidation;
};
}
}

#endif  // SRC_LIB_ACCESS_INDEXAWARETABLESCAN_SCAN
