// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/TableLoad.h"
#include "io/shortcuts.h"
#include "storage/RawTable.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class TpccTableLoadTests : public AccessTest {};
uint tableSize(std::string tableName){
    TableLoad tl;
    tl.setFileName("tpcc/"+tableName+".csv");
    tl.setDelimiter(",");
    tl.setHeaderFileName("tpcc/"+tableName+"_header.tbl");
    tl.setTableName("tpcc_"+tableName);
    tl.execute();

    const auto &result = tl.getResultTable();
    return result->size();
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_customer) {
  ASSERT_EQ(45u,tableSize("customer"));
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_district) {
  ASSERT_EQ(20u,tableSize("district"));
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_history) {
  ASSERT_EQ(45u,tableSize("history"));
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_item) {
  ASSERT_EQ(15u,tableSize("item"));
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_new_order) {
  ASSERT_EQ(45u,tableSize("new_order"));
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_order_line) {
  ASSERT_EQ(225u,tableSize("order_line"));
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_orders) {
  ASSERT_EQ(45u,tableSize("orders"));
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_stock) {
  ASSERT_EQ(30u,tableSize("stock"));
}

TEST_F(TpccTableLoadTests, table_load_with_header_file_test_warehouse) {
  ASSERT_EQ(2u,tableSize("warehouse"));
}

}
}
