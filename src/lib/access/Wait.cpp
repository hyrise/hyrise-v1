#include "access/Wait.h"

namespace hyrise { namespace access {

namespace { auto _ = QueryParser::registerPlanOperation<Wait>("Wait"); }

Wait::Wait(std::chrono::milliseconds wait) : _wait(wait) {}

void Wait::executePlanOperation() { std::this_thread::sleep_for(_wait); }

std::shared_ptr<PlanOperation> Wait::parse(const Json::Value& data) {
  return std::make_shared<Wait>(std::chrono::milliseconds(data.get("milliseconds", 0).asLargestUInt()));
}

}}
