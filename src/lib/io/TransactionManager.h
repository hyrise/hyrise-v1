// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_TRANSACTIONMANAGER_H_
#define SRC_LIB_IO_TRANSACTIONMANAGER_H_

#include <atomic>

#include "helper/locking.h"
#include "helper/types.h"

#include "io/TXContext.h"

#include <storage/storage_types.h>

#include <unordered_map>

namespace hyrise {
namespace tx {

// Stores all modifications for a given transaction
struct TXModifications {

  // Map type to store position list per table
  using map_t = std::unordered_map<uintptr_t, pos_list_t>;

  // TID identifier for the context
  transaction_id_t tid = UNKNOWN;

  // Map to store the values
  map_t inserted;
  map_t deleted;

  TXModifications(){}

  explicit TXModifications(transaction_id_t t) : tid(t) {}


  // Keeps track of all inserted rows
  void insertPos(const storage::c_atable_ptr_t& tab, pos_t pos) {
    static locking::Spinlock _mtx;
    _handle(_mtx, inserted, reinterpret_cast<uintptr_t>(tab.get()), pos);
  }

  // Keeps track of all deleted rows
  void deletePos(const storage::c_atable_ptr_t& tab, pos_t pos) {
    static locking::Spinlock _mtx;
    _handle(_mtx, deleted, reinterpret_cast<uintptr_t>(tab.get()), pos);
  }

  bool hasDeleted(const storage::c_atable_ptr_t& tab) const {
    return handleCheck(deleted, tab);
  }

  bool hasInserted(const storage::c_atable_ptr_t& tab) const {
    return handleCheck(inserted, tab);
  }

  const pos_list_t& getInserted(const storage::c_atable_ptr_t& tab) const {
    return inserted.at(reinterpret_cast<uintptr_t>(tab.get()));
  }

  const pos_list_t& getDeleted(const storage::c_atable_ptr_t& tab) const {
    return deleted.at(reinterpret_cast<uintptr_t>(tab.get()));
  }

private:

  bool handleCheck(const map_t& data, const storage::c_atable_ptr_t& tab) const {
    if (data.size() == 0)
      return false;

    auto key = reinterpret_cast<uintptr_t>(tab.get());
    if (data.find(key) != data.end() && data.at(key).size() > 0)
      return true;
    else 
      return false;
  }

  // Abstraction to the specific inserted and deleted row processes. To make
  // it convenient we pass all relevant data
  void _handle(locking::Spinlock& mtx, map_t& data, const uintptr_t& key, pos_t pos) {
    locking::ScopedLock<locking::Spinlock> lck(mtx);
    if(data.find(key) == data.end()) {
      data[key] = pos_list_t();
    }
    data[key].push_back(pos);
  }


};


/// Basic transaction manager
class TransactionManager {
  
  std::atomic<transaction_id_t> _transactionCount;

  std::atomic<transaction_cid_t> _commitId;

  using map_t = std::unordered_map<transaction_id_t, TXModifications>;

  // Keeping track of all transactions and their modifications
  map_t _txData;


  // Spin Lock for transactions
  hyrise::locking::Spinlock _txLock;

private:
  TransactionManager();

  // Get next transaction id
  transaction_id_t getTransactionId();
  

  

public:

  
  // Singleton Constructor
  static TransactionManager& getInstance();

  // get the last valid commit id for visibility
  inline transaction_id_t getLastCommitId() {
    return _commitId;
  }

  /*
  * Builds the transaction context by fetching the new transaction id and the
  * last commit id
  */
  TXContext buildContext() {
    TXContext ctx(getTransactionId(), getLastCommitId());
    return std::move(ctx);
  }

  /*
  * Starts the Synchronized Transaction Process
  *
  * The call to prepare commit retrieves the next possible commit ID from the
  * list and locks the call to serialized all incoming transactions. We
  * protect this method using a spin lock. The lock is released in the
  * @commit() method.
  */
  inline transaction_cid_t prepareCommit() {
    transaction_id_t result;
    while((result = tryPrepareCommit()) == 0) {
      std::this_thread::yield();
    }
    return result;
  }

  /**
  * Tries to acquire the spin lock for the prepare commit call and returns
  * hyrise::tx::UNKNOWN in case of failure or the next commit ID in case of
  * success
  */
  inline transaction_cid_t tryPrepareCommit() {
    if (_txLock.tryLock()){
      return getLastCommitId() + 1;
    }
    return hyrise::tx::UNKNOWN_CID;
  }


  /*
  * Returns the modifications set for the given transaction id
  */
  TXModifications& operator[](const transaction_id_t& key) {
#ifdef EXPENSIVE_ASSERTIONS
    if (_txData.find(key) == _txData.end())
      throw std::runtime_error("Retrieving Modification Set without initializing it first.");
#endif
    return _txData[key];
  }

  /**
  * This call relases the prepare commit lock and increments the commit ID
  * counter;
  */
  inline void commit(transaction_id_t tid) {
    if (!_txLock.isLocked()) 
      throw std::runtime_error("Double commit detected, possible TX corruption");
    
    ++_commitId;


    // Clear all relevant data for this transaction
    _txData.erase(tid);

    _txLock.unlock();
  }

  void reset();
};

}}

#endif
