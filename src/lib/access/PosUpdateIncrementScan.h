#ifndef SRC_LIB_ACCESS_POSUPDATEINCREMENTSCAN_H_
#define SRC_LIB_ACCESS_POSUPDATEINCREMENTSCAN_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class PosUpdateIncrementScan : public PlanOperation {
 public:
  PosUpdateIncrementScan(std::string column, Json::Value offset);
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

 private:
  const std::string _column;
  const Json::Value _offset;
};
}
}

#endif
