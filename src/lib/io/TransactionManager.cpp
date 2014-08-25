// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/TransactionManager.h"
#include "io/logging.h"
#include "io/StorageManager.h"
#include "io/GroupCommitter.h"
#include <cassert>
#include <limits>
#include <stdexcept>
#include <map>
#include <chrono>
#include <thread>

#include "optional.hpp"
#include "helper/make_unique.h"
#include "helper/checked_cast.h"
#include "helper/vector_helpers.h"
#include "helper/cas.h"
#include "storage/Store.h"

#define MAX_INFLIGHT_SIZE 1024 * 1024

namespace hyrise {
namespace tx {

void TXModifications::insertPos(const storage::c_atable_ptr_t& tab, pos_t pos) {
  _handle(_insert_mtx, inserted, tab, pos);
}

void TXModifications::deletePos(const storage::c_atable_ptr_t& tab, pos_t pos) {
  _handle(_delete_mtx, deleted, tab, pos);
}

bool TXModifications::hasDeleted(const storage::c_atable_ptr_t& tab) const { return handleCheck(deleted, tab); }

bool TXModifications::hasInserted(const storage::c_atable_ptr_t& tab) const { return handleCheck(inserted, tab); }

const pos_list_t& TXModifications::getInserted(const storage::c_atable_ptr_t& tab) const { return inserted.at(tab); }

const pos_list_t& TXModifications::getDeleted(const storage::c_atable_ptr_t& tab) const { return deleted.at(tab); }

bool TXModifications::handleCheck(const map_t& data, const storage::c_atable_ptr_t& tab) const {
  auto it = data.find(tab);
  return (it != data.end() && it->second.size() > 0);
}

void TXModifications::_handle(locking::Spinlock& mtx, map_t& data, const storage::c_atable_ptr_t& key, pos_t pos) {
  std::lock_guard<locking::Spinlock> lck(mtx);
  if (data.find(key) == data.end()) {
    data[key] = pos_list_t();
  }
  data[key].push_back(pos);
}

storage::store_ptr_t getStore(const storage::c_atable_ptr_t& table) {
  return std::const_pointer_cast<storage::Store>(checked_pointer_cast<const storage::Store>(table));
}

TransactionManager::TransactionManager() : _transactionCount(ATOMIC_VAR_INIT(tx::START_TID)) {
  _lastTXCommitContext = new TXCommitContext;
  _lastFinishedCommitId = tx::UNKNOWN_CID;
  _nextCommitId = ATOMIC_VAR_INIT(_lastFinishedCommitId + 1);
}

TransactionManager::~TransactionManager() {}

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

transaction_cid_t TransactionManager::getLastCommitId() { return _lastFinishedCommitId; }

TXContext TransactionManager::buildContext() {
  return {getTransactionId(), getLastCommitId()};
}

void TransactionManager::abort() {}


TXModifications& TransactionManager::operator[](const transaction_id_t& key) {
  return _txData([&key](map_t & txData)->TXModifications & {
    if (txData.find(key) == txData.end()) {
      txData[key] = make_unique<TransactionData>();
    }
    return txData[key]->_modifications;
  });
}

std::optional<const TXModifications&> TransactionManager::getModifications(const transaction_id_t key) const {
  return _txData([&key](const map_t & txData)->std::optional<const TXModifications&> {
    auto it = txData.find(key);
    if (it == txData.end()) {
      return std::nullopt;
    }
    return it->second->_modifications;
  });
}

void TransactionManager::reset() {
  _lastTXCommitContext = new TXCommitContext;
  _transactionCount = START_TID;
  _nextCommitId = UNKNOWN_CID;
  _lastFinishedCommitId = UNKNOWN_CID;
  _txData([](map_t& txData) { txData.clear(); });
}


TXContext TransactionManager::beginTransaction() { return getInstance().buildContext(); }

std::vector<TXContext> TransactionManager::getCurrentModifyingTransactionContexts() {
  return getInstance()._txData([](const map_t& data) {
    std::vector<TXContext> result;
    for (const auto& kv : data) {
      result.push_back(kv.second->_context);
    }
    return result;
  });
}


bool TransactionManager::isValidTransactionId(transaction_id_t tid) { return tid <= getInstance()._transactionCount; }

void TransactionManager::endTransaction(transaction_id_t tid, bool flush_log) {



#ifdef PERSISTENCY_BUFFEREDLOGGER
  // log commit entry and flush the log
  io::Logger::getInstance().logCommit(tid);
  if (flush_log)
    io::Logger::getInstance().flush();
#endif

  // Clear all relevant data for this transaction
  _txData([&tid](map_t& txData) { txData.erase(tid); });
}

void TransactionManager::rollbackTransaction(TXContext ctx) {
  // unmark positions previously marked for delete
  // auto& txData = getTransactionData(ctx.tid);
  for (auto& kv : getInstance()[ctx.tid].deleted) {
    auto store = getStore(kv.first.lock());
    store->unmarkForDeletion(kv.second, ctx.tid);
  }

  getInstance().endTransaction(ctx.tid, false);

#ifdef PERSISTENCY_BUFFEREDLOGGER
  io::Logger::getInstance().logRollback(ctx.tid);
#endif
}

void TransactionManager::commitModifiedPositions(TXContext& ctx, bool flush_log) {

  auto& txmgr = getInstance();
  if (auto mods = txmgr.getModifications(ctx.tid)) {
    const auto& modifications = *mods;


    for (auto& kv : modifications.inserted) {
      auto weak_table = kv.first;
      if (auto store = getStore(weak_table.lock())) {
        store->commitPositions(kv.second, ctx.cid, true);
      }
    }

    for (auto& kv : modifications.deleted) {
      auto weak_table = kv.first;
      if (auto store = getStore(weak_table.lock())) {
        store->commitPositions(kv.second, ctx.cid, false);
      }
    }
  }


#ifdef PERSISTENCY_BUFFEREDLOGGER
  io::Logger::getInstance().logCommit(ctx.tid);
  if (flush_log)
    io::Logger::getInstance().flush();
#endif
}

void TransactionManager::commitPendingTransactions(std::vector<TXCommitContext*>& commit_context_list,
                                                   TXCommitContext* commit_context) {

  // add ourself to the list
  commit_context_list.push_back(commit_context);

  // iterate over list of contexts until end or still running tx found
  auto current_commit_context = commit_context->next;
  while (current_commit_context != nullptr && current_commit_context->finished_but_uncommitted == true) {
    // try incrementing last finished cid
    auto previous_cid = current_commit_context->cid - 1;
    if (atomic_cas(&_lastFinishedCommitId, previous_cid, current_commit_context->cid)) {
      // tx committed successfully
      // add to list
      commit_context_list.push_back(current_commit_context);
      current_commit_context = current_commit_context->next;
      continue;
    } else {
      // could not increment last visible tx id
      // someone else must have sneaked in
      // so we stop here with our commit dependencies
      break;
    }
  }
}

TXCommitContext* TransactionManager::startCommitPhase(TXContext& ctx) {

  auto commit_context = new TXCommitContext(ctx);

  // ref count of commit_context is per default one, increment it
  // as next pointer from list will point to us
  commit_context->retain();

  // try until we succeed
  while (true) {
    // integrate us in the linked list
    if (atomic_cas<TXCommitContext*>(&_lastTXCommitContext->next, nullptr, commit_context)) {

      // increment ref count again as the last commit context pointer will point to us
      commit_context->retain();

      commit_context->cid = _lastTXCommitContext->cid + 1;
      ctx.cid = commit_context->cid;
      auto tmp = _lastTXCommitContext;
      _lastTXCommitContext = commit_context;

      // release prev commit context
      tmp->release();

      return commit_context;
    }
  }
}

std::vector<TXCommitContext*> TransactionManager::finishCommitPhase(TXCommitContext* commit_context, bool flush_log) {

  std::vector<TXCommitContext*> commit_tx_list;
  auto previous_cid = commit_context->cid - 1;

  // try incrementing last finished cid
  if (atomic_cas(&_lastFinishedCommitId, previous_cid, commit_context->cid)) {

    // everything went fine and the tx was committed
    // make sure to commit all pending transactions
    commitPendingTransactions(commit_tx_list, commit_context);
  } else {

    // incrementing last finished cid failed.
    // this means we are ahead of some other transactions and need to wait
    // so we mark ourself as finished but not committed
    commit_context->finished_but_uncommitted = true;

    // to make sure that the transaction we are waiting for has not finished
    // before we could add our tx to the list of pending transactions we try
    // incrementing the last finished cid again
    if (atomic_cas(&_lastFinishedCommitId, previous_cid, commit_context->cid)) {
      // it worked with the second try, so know the tx is committed
      // make sure to commit all pending transactions
      commit_context->finished_but_uncommitted = false;
      commitPendingTransactions(commit_tx_list, commit_context);

      // unmark oursefinished_but_uncommitted = false;
    } else {
    }
  }

  // flush cashes or log and clear tx data
  endTransaction(commit_context->tid, flush_log);

  return std::move(commit_tx_list);
}

std::vector<TXCommitContext*> TransactionManager::commitTransaction(TXContext ctx,
                                                                    bool flush_log,
                                                                    net::AbstractConnection* connection,
                                                                    std::string response) {

  if (!isValidTransactionId(ctx.tid)) {
    throw std::runtime_error("Transaction is not currently running");
  }

  auto& txmgr = getInstance();

  // get a commit id
  auto commit_context = txmgr.startCommitPhase(ctx);
  commit_context->connection = connection;
  commit_context->response = response;

  // first write all commit ids
  txmgr.commitModifiedPositions(ctx, flush_log);

  // finish commit phase
  return std::move(txmgr.finishCommitPhase(commit_context, flush_log));
}


void TransactionManager::commitAndRespond(TXContext& ctx,
                                          bool _use_group_commit,
                                          net::AbstractConnection* connection,
                                          std::string response) {

  // execute commit and get a list of all commit contexts we need to take care of and send the response
  auto commit_context_list = commitTransaction(ctx, !_use_group_commit, connection, response);

  for (const auto& tx_commit_context : commit_context_list) {
    if (_use_group_commit) {
      io::GroupCommitter::getInstance().push(std::tuple<net::AbstractConnection*, size_t, std::string>(
          tx_commit_context->connection, 200, tx_commit_context->response));
    } else {
      if (tx_commit_context->connection != nullptr) {
        tx_commit_context->connection->respond(tx_commit_context->response, 200);
      }
    }

    // release the context
    tx_commit_context->release();
  }
}


void TransactionManager::waitForAllCurrentlyRunningTransactions() const {
  std::vector<transaction_id_t> waiting_for_tx;

  // remember all currently running transactions
  _txData([&waiting_for_tx](map_t& txData) {
    for (const auto& kv : txData) {
      waiting_for_tx.push_back(kv.first);
    }
  });

  // block unitl all remembered transactions are finished
  while (true) {
    // remove all finished tx from waitlist
    // tx is finished if we can not find it transaction data
    auto new_end =
        std::remove_if(waiting_for_tx.begin(), waiting_for_tx.end(), [this](const transaction_id_t & tid)->bool {
          return _txData([&tid](map_t & txData)->bool { return txData.find(tid) == txData.end(); });
        });
    waiting_for_tx.erase(new_end, waiting_for_tx.end());

    // all finished?
    if (!waiting_for_tx.empty()) {
      std::chrono::nanoseconds sleep_time(50000000);  // 50 milliseconds
      std::this_thread::sleep_for(sleep_time);
    } else {
      // all tx have finihsed
      break;
    }
  }
}
}
}
