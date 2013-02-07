// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/TransactionManager.h"
#include <limits>
#include <stdexcept>

namespace hyrise {
namespace tx {

std::atomic<transaction_id_t> TransactionManager::_transactionCount;

TransactionManager& TransactionManager::getInstance() {
  static TransactionManager tm;
  return tm;
}

transaction_id_t TransactionManager::getTransactionId() {
  if (_transactionCount == std::numeric_limits<transaction_id_t>::max()) {
    throw std::runtime_error("Out of transaction ids - reached maximum");
  }
  return ++_transactionCount;

}

void TransactionManager::reset() {
  _transactionCount = START_TID;
}

}}
