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
#include "net/AbstractConnection.h"

#include "optional.hpp"


namespace hyrise {
namespace tx {

class TXCommitContext {
 public:
  transaction_id_t tid = UNKNOWN;
  transaction_cid_t cid = UNKNOWN;
  std::string response;
  net::AbstractConnection* connection;
  std::atomic<bool> finished_but_uncommitted;
  std::atomic<size_t> ref_count;
  TXCommitContext* next __attribute__((aligned(16)));

  TXCommitContext(TXContext& ctx)
      : tid(ctx.tid), cid(UNKNOWN), response(""), finished_but_uncommitted(false), ref_count(1), next(nullptr) {}

  TXCommitContext()
      : tid(UNKNOWN), cid(UNKNOWN), response(""), finished_but_uncommitted(false), ref_count(1), next(nullptr) {}

  ~TXCommitContext() {
    if (next != nullptr) {
      next->release();
    }
  }

  void retain() { ++ref_count; }

  void release() {
    assert(ref_count > 0);
    if (ref_count.fetch_sub(1) == 1) {
      delete this;
    }
  }
};


// Stores all modifications for a given transaction
class TXModifications {
 public:
  // Map type to store position list per table
  using map_t = std::map<std::weak_ptr<const storage::AbstractTable>,
                         storage::pos_list_t,
                         std::owner_less<std::weak_ptr<const storage::AbstractTable>>>;

  // TID identifier for the context
  transaction_id_t tid = UNKNOWN;



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

  // Map to store the values
  map_t inserted;
  map_t deleted;

 private:
  locking::Spinlock _insert_mtx;
  locking::Spinlock _delete_mtx;
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
  static std::vector<TXCommitContext*> commitTransaction(TXContext ctx,
                                                         bool _use_group_commit = true,
                                                         net::AbstractConnection* connection = nullptr,
                                                         std::string response = "");


  static void commitAndRespond(TXContext& ctx,
                               bool flush_log = true,
                               net::AbstractConnection* connection = nullptr,
                               std::string response = "");

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


  void abort();

  /*
  * Returns the modifications set for the given transaction id
  */
  TXModifications& operator[](const transaction_id_t& key);

  void endTransaction(transaction_id_t tid, bool flush_log);

  void reset();

  /*
  * Blocks until all currently running transactions have finished
  */
  void waitForAllCurrentlyRunningTransactions() const;


 private:
  std::optional<const TXModifications&> getModifications(const transaction_id_t key) const;
  void commitModifiedPositions(TXContext& ctx, bool flush_log);
  void commitPendingTransactions(std::vector<TXCommitContext*>& commit_context_list, TXCommitContext* commit_context);
  TXCommitContext* startCommitPhase(TXContext& ctx);
  std::vector<TXCommitContext*> finishCommitPhase(TXCommitContext* commit_context, bool flush_log);


  std::atomic<transaction_id_t> _transactionCount;
  std::atomic<transaction_cid_t> _nextCommitId;

  transaction_cid_t _lastFinishedCommitId __attribute__((aligned(16)));
  TXCommitContext* _lastTXCommitContext __attribute__((aligned(16)));

  using map_t = std::unordered_map<transaction_id_t, std::unique_ptr<TransactionData>>;

  // Keeping track of all transactions and their modifications
  Synchronized<map_t, locking::Spinlock> _txData;

  TransactionManager();
  ~TransactionManager();

  // Get next transaction id
  transaction_id_t getTransactionId();
};
}
}
