// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <string>

#include "helper.h"

//#include <access.h>
#include <access/InsertScan.h>
#include <storage.h>
#include <io.h>
#include <io/shortcuts.h>
#include <io/TransactionManager.h>

namespace hyrise {
namespace access {

class WriteTests : public AccessTest {};

TEST_F(WriteTests, insert_test) {
  auto writeCtx = tx::TransactionManager::getInstance().buildContext(); 
  hyrise::storage::atable_ptr_t s = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  hyrise::storage::atable_ptr_t i = io::Loader::shortcuts::load("test/insert_one.tbl");

  InsertScan gs;
  gs.setTXContext(writeCtx);
  gs.addInput(s);
  gs.setInputData(i);

  const auto& result = gs.execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/lin_xxxs_insert.tbl");

  ASSERT_TRUE(result->contentEquals(reference));
}

TEST_F(WriteTests, DISABLED_update_test_one_col) {
  /*   hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxxs.tbl");
       hyrise::storage::atable_ptr_t u = Loader::shortcuts::load("test/update_col_1.tbl");

       storage::Store * s = new Store(t);

       UpdateScan us;
       EqualsExpression<int> *eq0 = new EqualsExpression<int>(t, 0, 0);
       us.addInput(s);
       us.setUpdateTable(u);
       us.setPredicate(eq0);

       hyrise::storage::atable_ptr_t result = us.execute()->getResultTable();

       hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/update_test_one_col.tbl");

       result->print();
       reference->print();

       // TODO make this good again
       ASSERT_FALSE(result->contentEquals(reference));

       delete reference;
       delete t;
       delete u;
       delete s;
       delete result; */
}

TEST_F(WriteTests, DISABLED_update_test_col_all) {
  /*    hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxxs.tbl");
        hyrise::storage::atable_ptr_t u = Loader::shortcuts::load("test/update_col_all.tbl");

        storage::Store * s = new Store(t);

        UpdateScan us;
        EqualsExpression<int> *eq0 = new EqualsExpression<int>(t, 0, 0);
        us.addInput(s);
        us.setUpdateTable(u);
        us.setPredicate(eq0);
        hyrise::storage::atable_ptr_t result = us.execute()->getResultTable();

        hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/update_test_col_all.tbl");

        ASSERT_TRUE(result->contentEquals(reference));

        delete reference;
        delete t;
        delete u;
        delete eq0;
        delete s;
        delete result; */
}


TEST_F(WriteTests, DISABLED_update_test_col_all_rows) {
  /*    hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxxs.tbl");
        hyrise::storage::atable_ptr_t u = Loader::shortcuts::load("test/update_col_all.tbl");

        storage::Store * s = new Store(t);

        UpdateScan us;
        TrueExp *truth = new TrueExp(t, 0);
        us.addInput(s);
        us.setUpdateTable(u);
        us.setPredicate(truth);
        hyrise::storage::atable_ptr_t result = us.execute()->getResultTable();

        hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/update_test_col_all_rows.tbl");
        ASSERT_TRUE(result->contentEquals(reference));

        delete reference;
        delete t;
        delete u;
        delete truth;
        delete result;
        delete s; */
}

TEST_F(WriteTests, DISABLED_update_func_all_rows) {
  /*    hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/lin_xxxs.tbl");
        storage::Store * s = new Store(t);

        UpdateScan us;
        us.addInput(s);
        AddUpdateFun<int> f(t, 0, 1);
        us.setUpdateFunction(&f);
        TrueExp truth(t, 0);
        us.setPredicate(&truth);
        hyrise::storage::atable_ptr_t result = us.execute()->getResultTable();

        hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/update_func_all_rows.tbl");
        ASSERT_TRUE(result->contentEquals(reference));

        delete reference;
        delete result;
        delete s; */
}

}
}

