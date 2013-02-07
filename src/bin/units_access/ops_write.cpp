#include "testing/test.h"
#include <string>

#include "helper.h"

//#include <access.h>
#include <access/InsertScan.h>
#include <storage.h>
#include <io.h>
#include <io/shortcuts.h>

namespace hyrise {
namespace access {

class WriteTests : public AccessTest {};

TEST_F(WriteTests, insert_test) {
  AbstractTable::SharedTablePtr s = Loader::shortcuts::load("test/lin_xxxs.tbl");
  AbstractTable::SharedTablePtr i = Loader::shortcuts::load("test/insert_one.tbl");

  InsertScan gs;
  gs.addInput(s);
  gs.setInputData(i);

  const auto& result = gs.execute()->getResultTable();
  const auto& reference = Loader::shortcuts::load("test/reference/lin_xxxs_insert.tbl");

  ASSERT_TRUE(result->contentEquals(reference));
}

TEST_F(WriteTests, DISABLED_update_test_one_col) {
  /*   AbstractTable::SharedTablePtr t = Loader::shortcuts::load("test/lin_xxxs.tbl");
       AbstractTable::SharedTablePtr u = Loader::shortcuts::load("test/update_col_1.tbl");

       Store * s = new Store(t);

       UpdateScan us;
       EqualsExpression<int> *eq0 = new EqualsExpression<int>(t, 0, 0);
       us.addInput(s);
       us.setUpdateTable(u);
       us.setPredicate(eq0);

       AbstractTable::SharedTablePtr result = us.execute()->getResultTable();

       AbstractTable::SharedTablePtr reference = Loader::shortcuts::load("test/reference/update_test_one_col.tbl");

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
  /*    AbstractTable::SharedTablePtr t = Loader::shortcuts::load("test/lin_xxxs.tbl");
        AbstractTable::SharedTablePtr u = Loader::shortcuts::load("test/update_col_all.tbl");

        Store * s = new Store(t);

        UpdateScan us;
        EqualsExpression<int> *eq0 = new EqualsExpression<int>(t, 0, 0);
        us.addInput(s);
        us.setUpdateTable(u);
        us.setPredicate(eq0);
        AbstractTable::SharedTablePtr result = us.execute()->getResultTable();

        AbstractTable::SharedTablePtr reference = Loader::shortcuts::load("test/reference/update_test_col_all.tbl");

        ASSERT_TRUE(result->contentEquals(reference));

        delete reference;
        delete t;
        delete u;
        delete eq0;
        delete s;
        delete result; */
}


TEST_F(WriteTests, DISABLED_update_test_col_all_rows) {
  /*    AbstractTable::SharedTablePtr t = Loader::shortcuts::load("test/lin_xxxs.tbl");
        AbstractTable::SharedTablePtr u = Loader::shortcuts::load("test/update_col_all.tbl");

        Store * s = new Store(t);

        UpdateScan us;
        TrueExp *truth = new TrueExp(t, 0);
        us.addInput(s);
        us.setUpdateTable(u);
        us.setPredicate(truth);
        AbstractTable::SharedTablePtr result = us.execute()->getResultTable();

        AbstractTable::SharedTablePtr reference = Loader::shortcuts::load("test/reference/update_test_col_all_rows.tbl");
        ASSERT_TRUE(result->contentEquals(reference));

        delete reference;
        delete t;
        delete u;
        delete truth;
        delete result;
        delete s; */
}

TEST_F(WriteTests, DISABLED_update_func_all_rows) {
  /*    AbstractTable::SharedTablePtr t = Loader::shortcuts::load("test/lin_xxxs.tbl");
        Store * s = new Store(t);

        UpdateScan us;
        us.addInput(s);
        AddUpdateFun<int> f(t, 0, 1);
        us.setUpdateFunction(&f);
        TrueExp truth(t, 0);
        us.setPredicate(&truth);
        AbstractTable::SharedTablePtr result = us.execute()->getResultTable();

        AbstractTable::SharedTablePtr reference = Loader::shortcuts::load("test/reference/update_func_all_rows.tbl");
        ASSERT_TRUE(result->contentEquals(reference));

        delete reference;
        delete result;
        delete s; */
}

}
}

