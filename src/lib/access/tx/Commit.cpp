#include "Commit.h"

#include "access/system/QueryParser.h"
#include "io/TransactionManager.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<Commit>("Commit");
}

void Commit::executePlanOperation() {
  tx::TransactionManager::commitTransaction(_txContext, _flush_log);
  for (const auto& x : input.getTables()) {
    addResult(x);
  }
}

void Commit::setFlushLog(bool flush_log) { _flush_log = flush_log; }

std::shared_ptr<PlanOperation> Commit::parse(const Json::Value& data) {
  auto c = std::make_shared<Commit>();
  c->setFlushLog(data["flush_log"].asBool());
  return c;
}
}
}
