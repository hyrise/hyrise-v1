#include "testing/test.h"

#include <limits>
#include <fstream>

#include "io/TransactionManager.h"

namespace hyrise {
namespace tx {


using TM = TransactionManager;

TEST(TX, begin_commit) {
  TXContext tx = TM::beginTransaction();
  EXPECT_GT(tx.tid, 0);
  EXPECT_EQ(tx.cid, UNKNOWN);
  auto commit_context_list = TM::commitTransaction(tx);

  EXPECT_EQ(commit_context_list.size(), 1);
  auto commit_context = commit_context_list[0];
  EXPECT_NE(commit_context->cid, UNKNOWN);
}

TEST(TX, commit_invalid) {
  TXContext tx = TM::beginTransaction();
  transaction_id_t oldtid = tx.tid;
  transaction_id_t invalid = std::numeric_limits<transaction_id_t>::max();
  EXPECT_FALSE(TM::isValidTransactionId(invalid)) << "id is larger than acceptable";
  tx.tid = invalid;
  EXPECT_THROW(TM::commitTransaction(tx), std::runtime_error);  // "Commiting an invalid txid works.";
  tx.tid = oldtid;
  TM::commitTransaction(tx);
}

TEST(TX, modifying_transactions) {
  EXPECT_EQ(TM::getCurrentModifyingTransactionContexts().size(), 0u);
  auto t1 = TM::beginTransaction();
  EXPECT_EQ(TM::getCurrentModifyingTransactionContexts().size(), 0u);
  TM::commitTransaction(t1);
  EXPECT_EQ(TM::getCurrentModifyingTransactionContexts().size(), 0u);
}

TEST(TX, rollback_transaction) {
  auto before = TM::getInstance().getLastCommitId();
  auto t1 = TM::beginTransaction();
  TM::rollbackTransaction(t1);
  auto after = TM::getInstance().getLastCommitId();
  EXPECT_EQ(before, after) << "No commits are made when doing a rollback";
}
}
}  // namespace hyrise::tx
