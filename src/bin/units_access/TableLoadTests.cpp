// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/TableLoad.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "storage/RawTable.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class TableLoadTests : public AccessTest {};

TEST_F(TableLoadTests, basic_table_load_test) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");

  TableLoad tl;
  tl.setFileName("lin_xxs.tbl");
  tl.setTableName("myTable");
  tl.execute();

  const auto &result = tl.getResultTable();

  ASSERT_TRUE(result->contentEquals(t));
}

TEST_F(TableLoadTests, table_load_existing_test) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto sm = io::StorageManager::getInstance();
  sm->loadTable("myTable2", t);

  TableLoad tl;
  tl.setTableName("myTable2");
  tl.execute();

  const auto &result = tl.getResultTable();

  ASSERT_TRUE(result->contentEquals(t));
}

TEST_F(TableLoadTests, raw_table_load_test) {
  TableLoad tl;
  tl.setFileName("lin_xxs.tbl");
  tl.setTableName("myTable3");
  tl.setRaw(true);
  tl.execute();

  const auto &result = tl.getResultTable();
  auto raw = std::dynamic_pointer_cast<const storage::RawTable>(result);

  ASSERT_NE(raw, nullptr);
}

TEST_F(TableLoadTests, table_load_with_string_header_test) {
  auto t = io::Loader::shortcuts::loadWithHeader("test/header/data.tbl", "test/header/header.data");

  std::string header = "col1_0|col1_1|col1_2|col1_3|col1_4|col1_5|col1_6|col1_7|col1_8|col1_9\n \
                        INTEGER|INTEGER|INTEGER|INTEGER|INTEGER|INTEGER|INTEGER|INTEGER|INTEGER|INTEGER\n \
                        0_R|0_R|0_R|0_R|0_R|0_R|0_R|0_R|0_R|0_R";

  TableLoad tl;
  tl.setFileName("header/data.tbl");
  tl.setHeaderString(header);
  tl.setTableName("myTable4");
  tl.execute();

  const auto &result = tl.getResultTable();

  ASSERT_TRUE(result->contentEquals(t));
}

TEST_F(TableLoadTests, table_load_with_header_file_test) {
  auto t = io::Loader::shortcuts::loadWithHeader("test/header/data.tbl", "test/header/header.data");

  TableLoad tl;
  tl.setFileName("header/data.tbl");
  tl.setHeaderFileName("header/header.data");
  tl.setTableName("myTable5");
  tl.execute();

  const auto &result = tl.getResultTable();

  ASSERT_TRUE(result->contentEquals(t));
}

}
}
