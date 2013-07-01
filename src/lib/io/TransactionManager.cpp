// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/TransactionManager.h"
#include <limits>
#include <stdexcept>

namespace hyrise {
namespace tx {

TransactionManager::TransactionManager() : 
	_transactionCount(ATOMIC_VAR_INIT(0)) {}

TransactionManager& TransactionManager::getInstance() {
  static TransactionManager tm;
  return tm;
}

transaction_id_t TransactionManager::getTransactionId() {
	static locking::Spinlock _mtx;
	locking::ScopedLock<locking::Spinlock> lck(_mtx);

  if (_transactionCount == std::numeric_limits<transaction_id_t>::max()) {
    throw std::runtime_error("Out of transaction ids - reached maximum");
  }

  auto key = ++_transactionCount;

  // Create a new entry in teh TX Modifications set
  _txData[key] = TXModifications(key);

  return key;
}

void TransactionManager::reset() {
  _transactionCount = START_TID;
  _commitId = UNKNOWN_CID;
  _txData.clear();
}

}}
