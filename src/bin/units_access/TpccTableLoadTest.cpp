// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/TableLoad.h"
#include "io/shortcuts.h"
#include "storage/RawTable.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class TpccTableLoadTests : public AccessTest {};
uint tableLoad(std::string tableName){
    TableLoad tl;
    tl.setFileName("tpcc/"+tableName+".tbl");
    tl.setDelimiter("|");
    tl.setHeaderFileName("tpcc/"+tableName+"_header.tbl");
    tl.setTableName("tpcc_"+tableName);
    tl.execute();

    const auto &result = tl.getResultTable();
    return result->size();
}

// Table: customer Lines in CSV: 10 (load from test/tpcc/ customer.tbl, customer_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_customer) {
  ASSERT_EQ(10u,tableLoad("customer"));
}

// Table: district Lines in CSV: 5 (load from test/tpcc/ district.tbl, district_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_district) {
  ASSERT_EQ(5u,tableLoad("district"));
}

// Table: history Lines in CSV: 10 (load from test/tpcc/ history.tbl, history_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_history) {
  ASSERT_EQ(10u,tableLoad("history"));
}

// Table: item Lines in CSV: 10 (load from test/tpcc/ item.tbl, item_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_item) {
  ASSERT_EQ(10u,tableLoad("item"));
}

// Table: new_order Lines in CSV: 10 (load from test/tpcc/ new_order.tbl, new_order_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_new_order) {
  ASSERT_EQ(10u,tableLoad("new_order"));
}

// Table: order_line Lines in CSV: 10 (load from test/tpcc/ order_line.tbl, order_line_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_order_line) {
  ASSERT_EQ(10u,tableLoad("order_line"));
}

// Table: orders Lines in CSV: 10 (load from test/tpcc/ orders.tbl, orders_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_orders) {
  ASSERT_EQ(10u,tableLoad("orders"));
}

// Table: stock Lines in CSV: 10 (load from test/tpcc/ stock.tbl, stock_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_stock) {
  ASSERT_EQ(10u,tableLoad("stock"));
}

// Table: warehouse Lines in CSV: 1 (load from test/tpcc/ warehouse.tbl, warehouse_header.tbl)
TEST_F(TpccTableLoadTests, table_load_with_header_file_test_warehouse) {
  ASSERT_EQ(1u,tableLoad("warehouse"));
}

}
}