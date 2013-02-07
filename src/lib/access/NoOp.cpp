#include "NoOp.h"
#include "QueryParser.h"
bool NoOp::is_registered = QueryParser::registerPlanOperation<NoOp>();

std::shared_ptr<_PlanOperation> NoOp::parse(Json::Value &data) {
  return std::make_shared<NoOp>();
}

void NoOp::executePlanOperation() {
}
