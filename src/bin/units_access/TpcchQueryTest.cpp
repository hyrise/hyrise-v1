// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "io/StorageManager.h"

#include "helper.h"

namespace hyrise {
namespace access {

class TpcchQueryTest : public AccessTest {};
TEST_F(TpcchQueryTest, analyt_query1) {
  StorageManager * sm = StorageManager::getInstance();

  sm->loadTableFileWithHeader("order_line", "tpcc/order_line.csv", "tpcc/order_line_header.tbl");
  sm->loadTableFile("expectedTable", "tpcch/query1_result.tbl");

  std::string query = loadFromFile("test/tpcch/query1.json");

  const auto& out = executeAndWait(query);

  ASSERT_TRUE(out != nullptr);

  ASSERT_TABLE_EQUAL(out, sm->getTable("expectedTable"));
}

}
}