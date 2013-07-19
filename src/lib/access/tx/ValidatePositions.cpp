#include "ValidatePositions.h"

#include <storage/PointerCalculator.h>
#include <access/system/QueryParser.h>
#include <io/TransactionManager.h>

#include <helper/checked_cast.h>

namespace hyrise { namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<ValidatePositions>("ValidatePositions");
}


void ValidatePositions::executePlanOperation() {
  const auto& tab = checked_pointer_cast<const PointerCalculator>(getInputTable(0));
	auto pc = std::const_pointer_cast<PointerCalculator>(tab);
  pc->validate(_txContext.tid, _txContext.lastCid);

  // Get Modifications
  const auto& modifications = hyrise::tx::TransactionManager::getInstance()[_txContext.tid];
  if (modifications.hasDeleted(pc))
    pc->remove(modifications.getDeleted(pc));
	addResult(getInputTable(0));
}

std::shared_ptr<PlanOperation> ValidatePositions::parse(Json::Value &data) {
	return std::make_shared<ValidatePositions>();
}

}}