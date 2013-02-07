#ifndef SRC_LIB_ACCESS_MERGETABLE_H_
#define SRC_LIB_ACCESS_MERGETABLE_H_

#include "access/PlanOperation.h"

class MergeTable : public _PlanOperation {
 public:
  MergeTable();
  virtual ~MergeTable();
  const std::string vname();
  static std::shared_ptr<_PlanOperation> parse(Json::Value& data);
  void executePlanOperation();
};

#endif /* SRC_LIB_ACCESS_MERGETABLE_H_ */
