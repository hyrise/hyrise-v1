#include "access/tx/Rollback.h"

#include "io/TransactionManager.h"

namespace hyrise {
namespace access {
namespace {
auto reg_rollb = QueryParser::registerTrivialPlanOperation<Rollback>("Rollback");
}

void Rollback::executePlanOperation() { tx::TransactionManager::rollbackTransaction(_txContext); }
}
}
