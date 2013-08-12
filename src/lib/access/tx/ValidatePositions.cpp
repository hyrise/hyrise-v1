#include "ValidatePositions.h"

#include <storage/PointerCalculator.h>
#include <access/system/QueryParser.h>
#include <io/TransactionManager.h>

#include <helper/checked_cast.h>
#include <log4cxx/logger.h>

namespace hyrise { namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<ValidatePositions>("ValidatePositions");
  log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan.ValidatePositions"));
}


void ValidatePositions::executePlanOperation() {
  LOG4CXX_DEBUG(logger, "Validating Positions with: " << _txContext.tid << "(tid) and " << _txContext.lastCid << "(lCID)");
  const auto& tab = checked_pointer_cast<const PointerCalculator>(getInputTable(0));
  const auto& store = tab->getActualTable();
	auto pc = std::const_pointer_cast<PointerCalculator>(tab);
  pc->validate(_txContext.tid, _txContext.lastCid);

  // Get Modifications
  const auto& modifications = hyrise::tx::TransactionManager::getInstance()[_txContext.tid];
  if (modifications.hasDeleted(store))
    pc->remove(modifications.getDeleted(store));
	addResult(getInputTable(0));
}

std::shared_ptr<PlanOperation> ValidatePositions::parse(Json::Value &data) {
	return std::make_shared<ValidatePositions>();
}

}}