#include "Delete.h"

#include <access/system/QueryParser.h>
#include <access/system/ResponseTask.h>

#include <helper/checked_cast.h>

#include <io/TransactionManager.h>

#include <storage/Store.h>
#include <storage/PointerCalculator.h>

#include <log4cxx/logger.h>

namespace hyrise { namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<DeleteOp>("Delete");
  log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan.Delete"));
}


void DeleteOp::executePlanOperation() {
	auto tab = checked_pointer_cast<const PointerCalculator>(input.getTable(0));
	auto store = std::const_pointer_cast<Store>(checked_pointer_cast<const Store>(tab->getActualTable()));

	// A delete is nothing more than marking the positions as deleted in the TX
	// Modifications record 
	auto& modRecord = tx::TransactionManager::getInstance()[_txContext.tid];
	for(const auto& p : *(tab->getPositions())) {
		LOG4CXX_DEBUG(logger, "Deleting row:" << p);
		modRecord.deletePos(tab->getActualTable(), p);
	}

	auto rsp = getResponseTask();
  if (rsp != nullptr)
    rsp->incAffectedRows(tab->getPositions()->size());

	addResult(getInputTable(0));
}

std::shared_ptr<PlanOperation> DeleteOp::parse(Json::Value &data) {
	return std::make_shared<DeleteOp>();
}

}}