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
	auto tab = checked_pointer_cast<const storage::PointerCalculator>(input.getTable(0));
	auto store = std::const_pointer_cast<storage::Store>(checked_pointer_cast<const storage::Store>(tab->getActualTable()));

	auto& txmgr = tx::TransactionManager::getInstance();

	// A delete is nothing more than marking the positions as deleted in the TX
	// Modifications record
	auto& modRecord = txmgr[_txContext.tid];
	for(const auto& p : *(tab->getPositions())) {
		LOG4CXX_DEBUG(logger, "Deleting row:" << p);
		bool deleteOk = store->markForDeletion(p, _txContext.tid) == tx::TX_CODE::TX_OK;
		if(!deleteOk) {
		  txmgr.rollbackTransaction(_txContext);
			throw std::runtime_error("Aborted TX because TID of other TX found");
		}
		modRecord.deletePos(tab->getActualTable(), p);
	}

	auto rsp = getResponseTask();
  if (rsp != nullptr)
    rsp->incAffectedRows(tab->getPositions()->size());

	addResult(getInputTable(0));
}

std::shared_ptr<PlanOperation> DeleteOp::parse(const Json::Value &data) {
	return std::make_shared<DeleteOp>();
}

}}
