#include "ValidatePositions.h"

#include <storage/PointerCalculator.h>
#include <storage/Store.h>
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

  // Allow to operate directly on the store
  if (std::dynamic_pointer_cast<const storage::Store>(getInputTable(0))) {

    const auto& tab = checked_pointer_cast<const storage::Store>(getInputTable(0));
    auto pc = tab->buildValidPositions(_txContext.lastCid, _txContext.tid);
    addResult(std::make_shared<storage::PointerCalculator>(tab, new pos_list_t(std::move(pc))));

  } else {
    // If it's no store it has to be a pointer calculator otherwise there is
    // no data structure to work with
    const auto& tab = checked_pointer_cast<const storage::PointerCalculator>(getInputTable(0));
    const auto& store = tab->getActualTable();
    auto pc = std::const_pointer_cast<storage::PointerCalculator>(tab);
    pc->validate(_txContext.tid, _txContext.lastCid);

    // Get Modifications
    const auto& modifications = tx::TransactionManager::getInstance()[_txContext.tid];
    if (modifications.hasDeleted(store))
      pc->remove(modifications.getDeleted(store));
    addResult(getInputTable(0));
  }

  
}

std::shared_ptr<PlanOperation> ValidatePositions::parse(const Json::Value &data) {
	return std::make_shared<ValidatePositions>();
}

}}
