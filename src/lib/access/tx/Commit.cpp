#include "Commit.h"

#include <helper/checked_cast.h>
#include <storage/Store.h>
#include <storage/PointerCalculator.h>
#include <access/system/QueryParser.h>
#include <io/TransactionManager.h>

namespace hyrise { namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<Commit>("Commit");
}

void Commit::executePlanOperation() {

	// Acquire the TX Lock
	auto& txmgr = hyrise::tx::TransactionManager::getInstance();
	_txContext.cid = txmgr.prepareCommit();

	// Get all tables
	auto tables = hyrise::functional::collect(input.getTables(),
		[](const hyrise::storage::c_atable_ptr_t& t) {
			return std::const_pointer_cast<Store>(checked_pointer_cast<const Store>(t));
		});

	const auto& modifications = hyrise::tx::TransactionManager::getInstance()[_txContext.tid];	

	// Only update the required positions
	for(auto& store : tables) {
		// Only deleted records have to be checked for validity as newly inserted
		// records will be always only written by us
		if (modifications.hasDeleted(store) && 
			(hyrise::tx::TX_CODE::TX_OK != store->checkCommitID(modifications.getDeleted(store), _txContext.lastCid))) {
			txmgr.abort();
			throw std::runtime_error("Aborted TX with Last Commit ID != New Commit ID");
		}	
	}
	
	// After this point we are sure that no one can interfere our commit on this table
	for(auto& store : tables) {
		if (modifications.hasInserted(store)) {
			auto result = store->updateCommitID(modifications.getInserted(store), _txContext.cid, true);
			if (result != hyrise::tx::TX_CODE::TX_OK) {
				txmgr.abort();
				throw std::runtime_error("Aborted TX with "); // TODO at return code to error message	
			}
			
		}
	
		if (modifications.hasDeleted(store)) {
			auto result = store->updateCommitID(modifications.getDeleted(store), _txContext.cid, false);
			if (result != hyrise::tx::TX_CODE::TX_OK) {
				txmgr.abort();
				throw std::runtime_error("Aborted TX with "); // TODO at return code to error message
			}
		}	
	}

	txmgr.commit(_txContext.tid);
	
}

std::shared_ptr<_PlanOperation> Commit::parse(Json::Value &data) {
	return std::make_shared<Commit>();
}



}}

