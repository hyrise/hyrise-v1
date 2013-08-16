#include "testing/test.h"

#include <limits>

#include "io/TransactionManager.h"

using namespace hyrise::tx;
using TM = TransactionManager;

TEST(TX, begin_commit) {
  TXContext tx = TransactionManager::beginTransaction();
  EXPECT_GT(tx.tid, 0);
  EXPECT_EQ(tx.cid, UNKNOWN);
  auto cid = TransactionManager::commitTransaction(tx.tid);
  EXPECT_NE(cid, UNKNOWN);
}

TEST(TX, commit_invalid) {
  transaction_id_t invalid = std::numeric_limits<transaction_id_t>::max();
  EXPECT_FALSE(TransactionManager::isRunningTransaction(invalid));
  EXPECT_ANY_THROW(TransactionManager::commitTransaction(invalid));
}

TEST(TX, active_transactions) {
  ASSERT_EQ(TransactionManager::getRunningTransactionContexts().size(), 0u);
  auto t1 = TransactionManager::beginTransaction();
  auto t2 = TransactionManager::beginTransaction();
  EXPECT_EQ(TransactionManager::getRunningTransactionContexts().size(), 2u);
  TransactionManager::commitTransaction(t1.tid);
  TransactionManager::commitTransaction(t2.tid);
}

TEST(TX, rollback_transaction) {

}
