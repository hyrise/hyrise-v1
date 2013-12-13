// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "helper.h"

#include <access/storage/TableLoad.h>
#include <access/storage/UnloadAll.h>
#include <io/StorageManager.h>

namespace hyrise {
namespace access {

class LoadTests : public AccessTest {};

TEST_F(LoadTests, simple_load_op) {
  TableLoad t;
  t.setTableName("groupme");
  t.setFileName("sort_test.tbl");
  const auto& r = t.execute()->getResultTable();
  ASSERT_TRUE((bool)r) << "A table should be returned";

  // Workaround. Else, "groupme" will be released to often at block end.
  t.output.getTables().clear();
}


TEST_F(LoadTests, simple_unloadall_op) {
  TableLoad t;
  t.setTableName("groupme");
  t.setFileName("sort_test.tbl");
  const auto& r = t.execute()->getResultTable();
  ASSERT_TRUE((bool)r) << "A table should be returned";

  auto sm = io::StorageManager::getInstance();

  ASSERT_TRUE(sm->size() > 0) << "A table should be loaded";

  UnloadAll u;
  // Workaround. Else, "groupme" will be released to often at block end.
  t.output.getTables().clear();
  u.execute();

  ASSERT_EQ(0u, sm->size()) << "Not tables should be left after unload";
}


TEST_F(LoadTests, simple_query_with_loadops) {
  std::string q = loadFromFile("test/json/simple_query_with_loadop.json");
  const auto& out = executeAndWait(q);
  ASSERT_TRUE((bool)out);
}

TEST_F(LoadTests, simple_query_with_two_loadops) {
  std::string q = loadFromFile("test/json/two_loadop_one_unload.json");
  const auto&  outone = executeAndWait(q);
  if(outone) {
    q = loadFromFile("test/json/unload_io.json");
    const auto& out = executeAndWait(q);
    ASSERT_TRUE(!out);
  }
}

TEST_F(LoadTests, simple_query_with_load_with_header) {
  std::string q = loadFromFile("test/json/simple_query_load_with_header.json");
  const auto& out = executeAndWait(q);
  ASSERT_TRUE((bool)out);
}

TEST_F(LoadTests, load_exception) {
  std::string q = loadFromFile("test/json/load_exception.json");
  ASSERT_THROW( {
      executeAndWait(q);
    }, std::runtime_error);
}

/*
  TEST_F( LoadTests, load_from_layout )
  {

  StorageManager* sm = StorageManager::getInstance();
  std::string table_description = "Mobile | ID | Name | Mail | Company | Phone | Org\nINTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER | INTEGER\n0_R | 1_R | 1_R | 1_R | 1_R | 1_R | 1_R";
  sm->loadTableFileWithStringHeader("mytab", "tables/10_col_only_data.tbl" , table_description, true);

  hyrise::storage::atable_ptr_t  t = sm->getTable("mytab");
  ASSERT_EQ(2u, t->partitionCount());
  sm->removeAll();
  }
*/

#ifdef WITH_MYSQL

TEST_F(LoadTests, sql_table_load_op) {
  std::string q = loadFromFile("test/json/mysql_table_load.json");
  const auto& out = executeAndWait(q);
  ASSERT_TRUE((bool)out);
}

#endif

}
}

