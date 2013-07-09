// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_TRANSACTIONMANAGER_H_
#define SRC_LIB_IO_TRANSACTIONMANAGER_H_

#include <atomic>
#include <unordered_map>

#include "helper/locking.h"
#include "helper/types.h"
#include "io/TXContext.h"
#include "storage/storage_types.h"

namespace hyrise {
namespace tx {

// Stores all modifications for a given transaction
class TXModifications {
 public:
  // Map type to store position list per table
  using map_t = std::unordered_map<uintptr_t, storage::pos_list_t>;

  // TID identifier for the context
  transaction_id_t tid = UNKNOWN;

  // Map to store the values
  map_t inserted;
  map_t deleted;

  TXModifications() {};

  explicit TXModifications(transaction_id_t t) : tid(t) {}

  // Keeps track of all inserted rows
  void insertPos(const storage::c_atable_ptr_t& tab, pos_t pos);

  // Keeps track of all deleted rows
  void deletePos(const storage::c_atable_ptr_t& tab, pos_t pos);

  bool hasDeleted(const storage::c_atable_ptr_t& tab) const;

  bool hasInserted(const storage::c_atable_ptr_t& tab) const;

  const storage::pos_list_t& getInserted(const storage::c_atable_ptr_t& tab) const;
  const storage::pos_list_t& getDeleted(const storage::c_atable_ptr_t& tab) const;

private:
  bool handleCheck(const map_t& data, const storage::c_atable_ptr_t& tab) const;

  // Abstraction to the specific inserted and deleted row processes.
  void _handle(locking::Spinlock& mtx, map_t& data, const uintptr_t& key, pos_t pos);
};


/// Basic transaction manager
class TransactionManager {
  std::atomic<transaction_id_t> _transactionCount;
  std::atomic<transaction_cid_t> _commitId;

  using map_t = std::unordered_map<transaction_id_t, TXModifications>;

  // Keeping track of all transactions and their modifications
  map_t _txData;


  // Spin Lock for transactions
  locking::Spinlock _txLock;

  TransactionManager();

  // Get next transaction id
  transaction_id_t getTransactionId();
public:
  // Singleton Constructor
  static TransactionManager& getInstance();

  // get the last valid commit id for visibility
  transaction_id_t getLastCommitId();

  /*
  * Builds the transaction context by fetching the new transaction id and the
  * last commit id
  */
  TXContext buildContext();

  /*
  * Starts the Synchronized Transaction Process
  *
  * The call to prepare commit retrieves the next possible commit ID from the
  * list and locks the call to serialized all incoming transactions. We
  * protect this method using a spin lock. The lock is released in the
  * @commit() method.
  */
  transaction_cid_t prepareCommit();

  void abort();
  /**
  * Tries to acquire the spin lock for the prepare commit call and returns
  * hyrise::tx::UNKNOWN in case of failure or the next commit ID in case of
  * success
  */
  transaction_cid_t tryPrepareCommit();

  /*
  * Returns the modifications set for the given transaction id
  */
  TXModifications& operator[](const transaction_id_t& key);

  /**
  * This call relases the prepare commit lock and increments the commit ID
  * counter;
  */
  void commit(transaction_id_t tid);
  void reset();
};

}}

#endif
