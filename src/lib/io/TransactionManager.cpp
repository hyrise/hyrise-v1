// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/TransactionManager.h"

#include <limits>
#include <stdexcept>

namespace hyrise {
namespace tx {

void TXModifications::insertPos(const storage::c_atable_ptr_t& tab, pos_t pos) {
  static locking::Spinlock _mtx;
  _handle(_mtx, inserted, reinterpret_cast<uintptr_t>(tab.get()), pos);
}

void TXModifications::deletePos(const storage::c_atable_ptr_t& tab, pos_t pos) {
  static locking::Spinlock _mtx;
  _handle(_mtx, deleted, reinterpret_cast<uintptr_t>(tab.get()), pos);
}

bool TXModifications::hasDeleted(const storage::c_atable_ptr_t& tab) const {
  return handleCheck(deleted, tab);
}

bool TXModifications::hasInserted(const storage::c_atable_ptr_t& tab) const {
  return handleCheck(inserted, tab);
}

const pos_list_t& TXModifications::getInserted(const storage::c_atable_ptr_t& tab) const {
  return inserted.at(reinterpret_cast<uintptr_t>(tab.get()));
}

const pos_list_t& TXModifications::getDeleted(const storage::c_atable_ptr_t& tab) const {
  return deleted.at(reinterpret_cast<uintptr_t>(tab.get()));
}

bool TXModifications::handleCheck(const map_t& data, const storage::c_atable_ptr_t& tab) const {
  if (data.size() == 0)
    return false;

  auto key = reinterpret_cast<uintptr_t>(tab.get());
  if (data.find(key) != data.end() && data.at(key).size() > 0)
    return true;
  else
    return false;
}

void TXModifications::_handle(locking::Spinlock& mtx, map_t& data, const uintptr_t& key, pos_t pos) {
  locking::ScopedLock<locking::Spinlock> lck(mtx);
  if(data.find(key) == data.end()) {
    data[key] = pos_list_t();
  }
  data[key].push_back(pos);
}

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

transaction_id_t TransactionManager::getLastCommitId() {
  return _commitId;
}

TXContext TransactionManager::buildContext() {
  TXContext ctx(getTransactionId(), getLastCommitId());
  return std::move(ctx);
}

transaction_cid_t TransactionManager::prepareCommit() {
  transaction_id_t result;
  while((result = tryPrepareCommit()) == 0) {
    std::this_thread::yield();
  }
  return result;
}

void TransactionManager::abort() {
  if (!_txLock.isLocked())
    throw std::runtime_error("Cannot abort a not running transaction.");
  _txLock.unlock();
}

transaction_cid_t TransactionManager::tryPrepareCommit() {
  if (_txLock.tryLock()){
    return getLastCommitId() + 1;
  }
  return hyrise::tx::UNKNOWN_CID;
}


TXModifications& TransactionManager::operator[](const transaction_id_t& key) {
#ifdef EXPENSIVE_ASSERTIONS
  if (_txData.find(key) == _txData.end())
    throw std::runtime_error("Retrieving Modification Set without initializing it first.");
#endif
  return _txData[key];
}

void TransactionManager::commit(transaction_id_t tid) {
  if (!_txLock.isLocked())
    throw std::runtime_error("Double commit detected, possible TX corruption");
  ++_commitId;

  // Clear all relevant data for this transaction
  _txData.erase(tid);
  _txLock.unlock();
}


void TransactionManager::reset() {
  _transactionCount = START_TID;
  _commitId = UNKNOWN_CID;
  _txData.clear();
}

}}
