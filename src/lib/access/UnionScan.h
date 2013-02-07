#ifndef SRC_LIB_ACCESS_UNIONSCAN_H_
#define SRC_LIB_ACCESS_UNIONSCAN_H_

#include <access/PlanOperation.h>


class UnionScan : public _PlanOperation {

 public:

  UnionScan() : _PlanOperation() {
    
  }

  virtual ~UnionScan() {
    
  }

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data) {
    return std::make_shared<UnionScan>();
  }

  static bool is_registered;

  static std::string name() {
    return "UnionScan";
  }

  const std::string vname() {
    return "UnionScan";
  }

};
#endif  // SRC_LIB_ACCESS_UNIONSCAN_H_
