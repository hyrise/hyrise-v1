#ifndef SRC_LIB_ACCESS_SIMPLETABLESCAN_H_
#define SRC_LIB_ACCESS_SIMPLETABLESCAN_H_

#include "PlanOperation.h"
#include "pred_SimpleExpression.h"

class SimpleTableScan : public _PlanOperation {
 public:
  SimpleTableScan(): _PlanOperation(), _comparator(nullptr) {
    
  }

  virtual ~SimpleTableScan() {
    if (_comparator)
      delete _comparator;
  }

  void setPredicate(SimpleExpression *c) {
    _comparator = c;
  }

  void setupPlanOperation();

  void executePlanOperation();

  static std::string name() {
    return "SimpleTableScan";
  }

  const std::string vname() {

    return "SimpleTableScan";
  }

  static bool is_registered;

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);


 private:

  // Static logger for this class
  static log4cxx::LoggerPtr logger;

  // Comparison Field
  SimpleExpression *_comparator;
};

#endif  // SRC_LIB_ACCESS_SIMPLETABLESCAN_H_

