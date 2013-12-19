// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <algorithm>
#include <atomic>
#include <map>
#include <mutex>
#include <unordered_map>

#include "helper/locking.h"
#include "helper/Synchronized.h"
#include "helper/types.h"
#include "io/TXContext.h"
#include "storage/storage_types.h"

#include "optional.hpp"
namespace hyrise {
namespace tx {

// Stores all modifications for a given transaction
class TXModifications {
 public:
  // Map type to store position list per table
  using map_t = std::map<std::weak_ptr<const storage::AbstractTable>,
                         storage::pos_list_t,
                         std::owner_less<std::weak_ptr<const storage::AbstractTable> > >;

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
  void _handle(locking::Spinlock& mtx, map_t& data, const storage::c_atable_ptr_t& key, pos_t pos);
};

typedef struct TXData {
  TXContext _context;
  TXModifications _modifications;
} TransactionData;

/// Transaction manager based on transaction contexts
class TransactionManager {
 public:
  /// \defgroup Transaction Control Mechanism
  /// Use these methods to control transactions
  /// @{

  /// Starts a new transaction context, creates TransactionData object
  /// to be accessed through the returned context's `tid`.
  static TXContext beginTransaction();

  /// Returns transaction data reference for modification
  /// \param tid transaction id
  static TransactionData& getTransactionData(transaction_id_t tid);
  /// Returns context for tid
  /// \param tid transaction id
  static TXContext getContext(transaction_id_t tid);

  /// Make all changes visible to other transaction, ending the lifetime
  /// of the transaction context identified by tid
  /// \param tid transaction id to commit
  /// \returns commit id on success
  static transaction_cid_t commitTransaction(TXContext ctx);

  /// Ends a transaction by leaving all changes invisible
  /// \param tid transaction id to abort
  static void rollbackTransaction(TXContext ctx);

  /// Check validity of a transactionId - this doesn't guarantee
  /// that the transaction is uncommitted
  /// \param tid transaction id under investigation
  static bool isValidTransactionId(transaction_id_t tid);
  static std::vector<TXContext> getCurrentModifyingTransactionContexts();
  /// @}

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
  * UNKNOWN in case of failure or the next commit ID in case of
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

  void endTransaction(transaction_id_t tid);

  void reset();


 private:
  std::optional<const TXModifications&> getModifications(const transaction_id_t key) const;

  std::atomic<transaction_id_t> _transactionCount;
  std::atomic<transaction_cid_t> _commitId;

  using map_t = std::unordered_map<transaction_id_t,
                                   std::unique_ptr<TransactionData>>;

  // Keeping track of all transactions and their modifications
  Synchronized<map_t, locking::Spinlock> _txData;

  // Spin Lock for transactions
  locking::Spinlock _txLock;

  TransactionManager();

  // Get next transaction id
  transaction_id_t getTransactionId();
};

}}

