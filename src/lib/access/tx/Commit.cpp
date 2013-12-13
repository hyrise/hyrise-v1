#include "Commit.h"

#include "access/system/QueryParser.h"
#include "io/TransactionManager.h"

namespace hyrise { namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<Commit>("Commit");
}

void Commit::executePlanOperation() {
  tx::TransactionManager::commitTransaction(_txContext);
  for(const auto& x : input.getTables()) {
    addResult(x);
  }
}

std::shared_ptr<PlanOperation> Commit::parse(const Json::Value &data) {
  return std::make_shared<Commit>();
}

}}
