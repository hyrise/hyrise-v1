// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/TransactionManager.h"
#include <cassert>
#include <limits>
#include <stdexcept>
#include <map>


#include "helper/make_unique.h"
#include "helper/checked_cast.h"
#include "helper/vector_helpers.h"
#include "storage/Store.h"

namespace hyrise {
namespace tx {

void TXModifications::insertPos(const storage::c_atable_ptr_t& tab, pos_t pos) {
  static locking::Spinlock _mtx;
  _handle(_mtx, inserted, tab, pos);
}

void TXModifications::deletePos(const storage::c_atable_ptr_t& tab, pos_t pos) {
  static locking::Spinlock _mtx;
  _handle(_mtx, deleted, tab, pos);
}

bool TXModifications::hasDeleted(const storage::c_atable_ptr_t& tab) const {
  return handleCheck(deleted, tab);
}

bool TXModifications::hasInserted(const storage::c_atable_ptr_t& tab) const {
  return handleCheck(inserted, tab);
}

const pos_list_t& TXModifications::getInserted(const storage::c_atable_ptr_t& tab) const {
  return inserted.at(tab);
}

const pos_list_t& TXModifications::getDeleted(const storage::c_atable_ptr_t& tab) const {
  return deleted.at(tab);
}

bool TXModifications::handleCheck(const map_t& data, const storage::c_atable_ptr_t& tab) const {
  auto it = data.find(tab);
  return (it != data.end() && it->second.size() > 0);
}

void TXModifications::_handle(locking::Spinlock& mtx, map_t& data, const storage::c_atable_ptr_t& key, pos_t pos) {
  std::lock_guard<locking::Spinlock> lck(mtx);
  if(data.find(key) == data.end()) {
    data[key] = pos_list_t();
  }
  data[key].push_back(pos);
}

TransactionManager::TransactionManager() :
    _transactionCount(ATOMIC_VAR_INIT(tx::START_TID)),
    _commitId(ATOMIC_VAR_INIT(tx::UNKNOWN_CID)) {}

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

transaction_cid_t TransactionManager::getLastCommitId() {
  return _commitId;
}

TXContext TransactionManager::buildContext() {
  TXContext ctx(getTransactionId(), getLastCommitId());
  _txData([&ctx](map_t& txData) {
      txData[ctx.tid] = make_unique<TransactionData>(ctx);
    });
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
  if (!_txLock.is_locked())
    throw std::runtime_error("Cannot abort a not running transaction.");
  _txLock.unlock();
}

transaction_cid_t TransactionManager::tryPrepareCommit() {
  _txLock.lock();
  return getLastCommitId() + 1;
}

TXModifications& TransactionManager::operator[](const transaction_id_t& key) {
  return _txData([&key] (map_t& txData) -> TXModifications& {
#ifdef EXPENSIVE_ASSERTIONS
      if (txData.find(key) == txData.end()) {
        throw std::runtime_error("Retrieving Modification Set without initializing it first.");
      }
#endif
      return txData[key]->_modifications;
    });
}

void TransactionManager::commit(transaction_id_t tid) {
  if (!_txLock.is_locked())
    throw std::runtime_error("Double commit detected, possible TX corruption");
  ++_commitId;
  _txLock.unlock();

  endTransaction(tid);
}


void TransactionManager::reset() {
  _transactionCount = START_TID;
  _commitId = UNKNOWN_CID;
  _txData([] (map_t& txData) { txData.clear(); });
}

TXContext TransactionManager::beginTransaction() {
  return getInstance().buildContext();
}

std::vector<TXContext> TransactionManager::getRunningTransactionContexts() {
  return getInstance()._txData([] (const map_t& data) {
      std::vector<TXContext> result;
      for(const auto& kv: data) {
        result.push_back(kv.second->_context);
      }
      return result;
    });
}

bool TransactionManager::isRunningTransaction(transaction_id_t tid) {
  return getInstance()._txData([&] (const map_t& data) {
      return data.find(tid) != data.end();
    });
}

TXContext TransactionManager::getContext(transaction_id_t tid) {
  return getInstance()._txData([&tid] (const map_t& data) {
      return data.at(tid)->_context;
    });
}

TransactionData& TransactionManager::getTransactionData(transaction_id_t tid) {
  return getInstance()._txData([&] (const map_t& txData) -> TransactionData& {
      return *txData.at(tid).get();
    });
}

storage::store_ptr_t getStore(const storage::c_atable_ptr_t& table) {
  return std::const_pointer_cast<storage::Store>(
      checked_pointer_cast<const storage::Store>(table));
}

void TransactionManager::endTransaction(transaction_id_t tid) {
  // Clear all relevant data for this transaction
  _txData([&tid] (map_t& txData) {
      std::size_t erased = txData.erase(tid);
      assert(erased == 1);
    });
}

void TransactionManager::rollbackTransaction(TXContext ctx) {
  if (!isRunningTransaction(ctx.tid)) {
    throw std::runtime_error("Transaction is not currently running");
  }

  // unmark positions previously marked for delete
  auto& txData = getTransactionData(ctx.tid);
  for(auto& kv : txData._modifications.deleted) {
    auto store = getStore(kv.first.lock());
    store->unmarkForDeletion(kv.second, ctx.tid);
  }

  getInstance().endTransaction(ctx.tid);
}

transaction_cid_t TransactionManager::commitTransaction(TXContext ctx) {
  if (!isRunningTransaction(ctx.tid)) {
    throw std::runtime_error("Transaction is not currently running");
  }
  auto& txmgr = getInstance();
  auto& tx_data = getTransactionData(ctx.tid);
  const auto& modifications = tx_data._modifications;

  ctx.cid = txmgr.prepareCommit();

  // Only update the required positions
  for (auto& kv: modifications.deleted) {
    auto weak_table = kv.first;
    // Only deleted records have to be checked for validity as newly inserted
    // records will be always only written by us
    if (auto store = getStore(weak_table.lock())) {
      if (TX_CODE::TX_OK != store->checkForConcurrentCommit(kv.second, ctx.tid)) {
        txmgr.abort();
        throw std::runtime_error("Aborted TX with Last Commit ID != New Commit ID");
      }
    }
  }

  for (auto& kv: modifications.inserted) {
    auto weak_table = kv.first;
    if (auto store = getStore(weak_table.lock())) {
      auto result = store->commitPositions(kv.second, ctx.cid, true);
      if (result != TX_CODE::TX_OK) {
        txmgr.abort();
        throw std::runtime_error("Aborted TX with "); // TODO at return code to error message
      }
    }
  }

  for (auto& kv: modifications.deleted) {
    auto weak_table = kv.first;
    if (auto store = getStore(weak_table.lock())) {
      auto result = store->commitPositions(kv.second, ctx.cid, false);
      if (result != TX_CODE::TX_OK) {
        txmgr.abort();
        throw std::runtime_error("Aborted TX with "); // TODO at return code to error message
      }
    }
  }

  txmgr.commit(ctx.tid);
  return ctx.cid;
}

}}

