// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/storage/TableLoad.h"
#include "io/shortcuts.h"
#include "storage/RawTable.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class TpccTableLoadTests : public AccessTest {};

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_customer) {
      // Testing load of TPC-C XXXS Tables .
      // Table: customer Lines in CSV: 10       
      // Load from test/tpcc/ customer.csv and customer_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/customer.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/customer_header.tbl");
      tl.setTableName("tpcc_customer");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(10u,result->size());
    }
    
    
    

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_district) {
      // Testing load of TPC-C XXXS Tables .
      // Table: district Lines in CSV: 10       
      // Load from test/tpcc/ district.csv and district_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/district.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/district_header.tbl");
      tl.setTableName("tpcc_district");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(10u,result->size());
    }
    
    
    

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_history) {
      // Testing load of TPC-C XXXS Tables .
      // Table: history Lines in CSV: 10       
      // Load from test/tpcc/ history.csv and history_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/history.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/history_header.tbl");
      tl.setTableName("tpcc_history");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(10u,result->size());
    }
    
    
    

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_item) {
      // Testing load of TPC-C XXXS Tables .
      // Table: item Lines in CSV: 10       
      // Load from test/tpcc/ item.csv and item_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/item.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/item_header.tbl");
      tl.setTableName("tpcc_item");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(10u,result->size());
    }
    
    
    

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_new_order) {
      // Testing load of TPC-C XXXS Tables .
      // Table: new_order Lines in CSV: 10       
      // Load from test/tpcc/ new_order.csv and new_order_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/new_order.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/new_order_header.tbl");
      tl.setTableName("tpcc_new_order");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(10u,result->size());
    }
    
    
    

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_order_line) {
      // Testing load of TPC-C XXXS Tables .
      // Table: order_line Lines in CSV: 10       
      // Load from test/tpcc/ order_line.csv and order_line_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/order_line.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/order_line_header.tbl");
      tl.setTableName("tpcc_order_line");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(10u,result->size());
    }
    
    
    

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_orders) {
      // Testing load of TPC-C XXXS Tables .
      // Table: orders Lines in CSV: 10       
      // Load from test/tpcc/ orders.csv and orders_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/orders.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/orders_header.tbl");
      tl.setTableName("tpcc_orders");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(10u,result->size());
    }
    
    
    

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_stock) {
      // Testing load of TPC-C XXXS Tables .
      // Table: stock Lines in CSV: 10       
      // Load from test/tpcc/ stock.csv and stock_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/stock.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/stock_header.tbl");
      tl.setTableName("tpcc_stock");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(10u,result->size());
    }
    
    
    

    TEST_F(TpccTableLoadTests, table_load_with_header_file_test_warehouse) {
      // Testing load of TPC-C XXXS Tables .
      // Table: warehouse Lines in CSV: 4       
      // Load from test/tpcc/ warehouse.csv and warehouse_header.tbl

      TableLoad tl;
      tl.setFileName("tpcc/warehouse.csv");
      tl.setDelimiter(",");
      tl.setHeaderFileName("tpcc/warehouse_header.tbl");
      tl.setTableName("tpcc_warehouse");
      tl.execute();

      const auto &result = tl.getResultTable();
      ASSERT_EQ(4u,result->size());
    }
}
}

