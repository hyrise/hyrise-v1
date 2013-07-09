#include "Delete.h"
#include <access/system/QueryParser.h>

#include <helper/checked_cast.h>

#include <io/TransactionManager.h>

#include <storage/Store.h>
#include <storage/PointerCalculator.h>


namespace hyrise { namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<DeleteOp>("Delete");
}


void DeleteOp::executePlanOperation() {
	auto tab = checked_pointer_cast<const PointerCalculator>(input.getTable(0));
	auto store = std::const_pointer_cast<Store>(checked_pointer_cast<const Store>(tab->getActualTable()));

	// A delete is nothing more than marking the positions as deleted in the TX
	// Modifications record 
	auto& modRecord = tx::TransactionManager::getInstance()[_txContext.tid];
	for(const auto& p : *(tab->getPositions())) {
		modRecord.deletePos(tab->getActualTable(), p);

		// This is bad as it can override other peoples delete that should fail later
		store->setTid(p, _txContext.tid);
	}
	addResult(getInputTable(0));
}

std::shared_ptr<_PlanOperation> DeleteOp::parse(Json::Value &data) {
	return std::make_shared<DeleteOp>();
}

}}