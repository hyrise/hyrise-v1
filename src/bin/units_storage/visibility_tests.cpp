// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"

#include <algorithm>

#include <io/shortcuts.h>
#include <storage/Store.h>
#include <storage/PointerCalculator.h>
#include <storage/TableBuilder.h>
#include <helper/types.h>
#include <io/TransactionManager.h>

#include <testing/TableEqualityTest.h>

namespace  hyrise { namespace storage {

class VisibilityTests : public ::hyrise::Test {

public:

	storage::c_atable_ptr_t data;
	storage::store_ptr_t linxxxs;

	storage::atable_ptr_t one_row;

	void SetUp() {
		 TableBuilder::param_list list;
		 list.append().set_type("INTEGER").set_name("a");
		 list.append().set_type("FLOAT").set_name("b");
		 list.append().set_type("STRING").set_name("c");

		 list.appendGroup(1).appendGroup(1).appendGroup(1);

		 TableBuilder::param_list list2;
		 list2.append().set_type("INTEGER").set_name("col_0");
		 list2.append().set_type("INTEGER").set_name("col_1");
		 one_row = TableBuilder::build(list2);

		 one_row->resize(1);
		 one_row->setValue<hyrise_int_t>(0,0, 99);
		 one_row->setValue<hyrise_int_t>(1,0, 999);

		 // Convert to store
		 data = std::make_shared<storage::Store>(storage::TableBuilder::build(list));
		 linxxxs = std::dynamic_pointer_cast<storage::Store>(io::Loader::shortcuts::load("test/lin_xxxs.tbl"));
	}

};

TEST_F(VisibilityTests, validate_records) {
	auto&	 txmgr = hyrise::tx::TransactionManager::getInstance();
	auto ctx = txmgr.buildContext();

	pos_list_t tmp(linxxxs->size(), 0);
	size_t i=0;
	std::generate(std::begin(tmp), std::end(tmp), [&i](){ return i++; });
	linxxxs->validatePositions(tmp, ctx.lastCid, ctx.tid);
	ASSERT_EQ(linxxxs->size(), tmp.size());
}

TEST_F(VisibilityTests, check_tx_prepare_commit) {
	auto&	 txmgr = hyrise::tx::TransactionManager::getInstance();
	auto ctx = txmgr.buildContext();
	auto lc = ctx.lastCid;

	ASSERT_EQ(hyrise::tx::UNKNOWN, lc);
	ASSERT_EQ(lc + 1, txmgr.tryPrepareCommit());
	txmgr.commit(ctx.tid);
	ASSERT_EQ(lc + 1, txmgr.getLastCommitId());
	ASSERT_ANY_THROW(txmgr.commit(hyrise::tx::UNKNOWN)) << "Double commit is not allowed";
}

TEST_F(VisibilityTests, read_your_own_writes) {
	auto&	 txmgr = hyrise::tx::TransactionManager::getInstance();

	auto ctx_a = txmgr.buildContext();
	auto ctx_b = txmgr.buildContext();

	auto tid_a = ctx_a.tid;
	auto tid_b = ctx_b.tid;
	auto lc = txmgr.getLastCommitId();

	linxxxs->resizeDelta(1);
	linxxxs->copyRowToDelta(one_row, 0, 0, tid_a);

	pos_list_t tmp(linxxxs->size(), 0);
	size_t i=0;
	std::generate(std::begin(tmp), std::end(tmp), [&i](){ return i++; });
	linxxxs->validatePositions(tmp, lc, tid_a);
	ASSERT_EQ(linxxxs->size(), tmp.size());


	// the second transaction should only see the base values
	auto tmp2 = new pos_list_t(linxxxs->size(), 0);
	i=0;
	std::generate(std::begin(*tmp2), std::end(*tmp2), [&i](){ return i++; });
	linxxxs->validatePositions(*tmp2, lc, tid_b);
	ASSERT_EQ(linxxxs->size()-1, tmp2->size());

	auto r = std::make_shared<PointerCalculator>(linxxxs, tmp2);
	EXPECT_RELATION_EQ(io::Loader::shortcuts::load("test/lin_xxxs.tbl"), r);
}

TEST_F (VisibilityTests, read_writes_after_commit) {
	auto&	 txmgr = hyrise::tx::TransactionManager::getInstance();
	auto ctx_a = txmgr.buildContext();
	auto ctx_b = txmgr.buildContext();

	auto tid_a = ctx_a.tid;
	auto tid_b = ctx_b.tid;
	auto lc = txmgr.getLastCommitId();

	linxxxs->resizeDelta(1);
	linxxxs->copyRowToDelta(one_row, 0, 0, tid_a);

	pos_list_t tmp(linxxxs->size(), 0);
	size_t i=0;
	std::generate(std::begin(tmp), std::end(tmp), [&i](){ return i++; });
	linxxxs->validatePositions(tmp, lc, tid_a);
	ASSERT_EQ(linxxxs->size(), tmp.size());

	auto next_cid = txmgr.prepareCommit();
	ASSERT_EQ(next_cid, txmgr.getLastCommitId() + 1);

	pos_list_t pos_tmp = {linxxxs->size() -1};
	ASSERT_EQ(hyrise::tx::TX_CODE::TX_OK, linxxxs->commitPositions(pos_tmp, next_cid, true));
	txmgr.commit(tid_a);

	// the second transaction should see all the values after the commit is done
	lc = txmgr.getLastCommitId();
	auto tmp2 = new pos_list_t(linxxxs->size(), 0);
	i=0;
	std::generate(std::begin(*tmp2), std::end(*tmp2), [&i](){ return i++; });
	linxxxs->validatePositions(*tmp2, lc, tid_b);
	ASSERT_EQ(linxxxs->size(), tmp2->size());

	auto r = std::make_shared<PointerCalculator>(linxxxs, tmp2);
	EXPECT_RELATION_EQ(linxxxs, r);
}

TEST_F (VisibilityTests, read_writes_after_commit_old_cid) {
	auto&	 txmgr = hyrise::tx::TransactionManager::getInstance();
	auto ctx_a = txmgr.buildContext();
	auto ctx_b = txmgr.buildContext();

	auto tid_a = ctx_a.tid;
	auto tid_b = ctx_b.tid;
	auto lc = txmgr.getLastCommitId();

	linxxxs->resizeDelta(1);
	linxxxs->copyRowToDelta(one_row, 0, 0, tid_a);

	pos_list_t tmp(linxxxs->size(), 0);
	size_t i=0;
	std::generate(std::begin(tmp), std::end(tmp), [&i](){ return i++; });
	linxxxs->validatePositions(tmp, lc, tid_a);
	ASSERT_EQ(linxxxs->size(), tmp.size());

	auto next_cid = txmgr.prepareCommit();
	ASSERT_EQ(next_cid, txmgr.getLastCommitId() + 1);
	pos_list_t pos_tmp = {linxxxs->size() -1};
	ASSERT_EQ(hyrise::tx::TX_CODE::TX_OK, linxxxs->commitPositions(pos_tmp, next_cid, true));
	txmgr.commit(tid_a);

	// the second transaction should not see all the values after the commit, due to old cid
	auto tmp2 = new pos_list_t(linxxxs->size(), 0);
	i=0;
	std::generate(std::begin(*tmp2), std::end(*tmp2), [&i](){ return i++; });
	linxxxs->validatePositions(*tmp2, lc, tid_b);
	ASSERT_EQ(linxxxs->size() -1, tmp2->size());

	auto r = std::make_shared<PointerCalculator>(linxxxs, tmp2);
	EXPECT_RELATION_EQ(io::Loader::shortcuts::load("test/lin_xxxs.tbl"), r);
}

}}
