#include "testing/test.h"

#include <limits>

#include "io/TransactionManager.h"

namespace hyrise {
namespace tx {


using TM = TransactionManager;

TEST(TX, begin_commit) {
  TXContext tx = TM::beginTransaction();
  EXPECT_GT(tx.tid, 0);
  EXPECT_EQ(tx.cid, UNKNOWN);
  auto cid = TM::commitTransaction(tx);
  EXPECT_NE(cid, UNKNOWN);
}

TEST(TX, commit_invalid) {
  TXContext tx = TM::beginTransaction();
  transaction_id_t oldtid = tx.tid;
  transaction_id_t invalid = std::numeric_limits<transaction_id_t>::max();
  EXPECT_FALSE(TM::isRunningTransaction(invalid));
  tx.tid = invalid;
  EXPECT_ANY_THROW(TM::commitTransaction(tx));
  tx.tid = oldtid;
  TM::commitTransaction(tx);
}

TEST(TX, active_transactions) {
  ASSERT_EQ(TM::getRunningTransactionContexts().size(), 0u);
  auto t1 = TM::beginTransaction();
  auto t2 = TM::beginTransaction();
  EXPECT_EQ(TM::getRunningTransactionContexts().size(), 2u);
  TM::commitTransaction(t1);
  TM::commitTransaction(t2);
  EXPECT_EQ(TM::getRunningTransactionContexts().size(), 0u);
}

TEST(TX, rollback_transaction) {
  auto before = TM::getInstance().getLastCommitId();
  ASSERT_EQ(TM::getRunningTransactionContexts().size(), 0u);
  auto t1 = TM::beginTransaction();
  TM::rollbackTransaction(t1);
  EXPECT_EQ(TM::getRunningTransactionContexts().size(), 0u);
  auto after = TM::getInstance().getLastCommitId();
  EXPECT_EQ(before, after) << "No commits are made when doing a rollback";
}

} } // namespace hyrise::tx

