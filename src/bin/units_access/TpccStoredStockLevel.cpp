// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TpccStoredProceduresTest.h"

namespace hyrise {
namespace access {

class TpccStoredStockLevelTest : public TpccStoredProceduresTest {

 protected:
  Json::Value doStockLevel(int w_id, int d_id, int threshold);
};
#define T_StockLevel(w_id, d_id, threshold)                    \
  {                                                            \
    const auto response = doStockLevel(w_id, d_id, threshold); \
                                                               \
    EXPECT_EQ(w_id, getValuei(response, "W_ID"));              \
    EXPECT_EQ(d_id, getValuei(response, "D_ID"));              \
    EXPECT_EQ(threshold, getValuei(response, "threshold"));    \
    EXPECT_EQ(0, getValuei(response, "low_stock"));            \
                                                               \
    EXPECT_EQ(getTable(Customer)->size(), i_customer_size);    \
    EXPECT_EQ(getTable(Orders)->size(), i_orders_size);        \
    EXPECT_EQ(getTable(OrderLine)->size(), i_orderLine_size);  \
    EXPECT_EQ(getTable(Warehouse)->size(), i_warehouse_size);  \
    EXPECT_EQ(getTable(NewOrder)->size(), i_newOrder_size);    \
    EXPECT_EQ(getTable(District)->size(), i_district_size);    \
    EXPECT_EQ(getTable(Item)->size(), i_item_size);            \
    EXPECT_EQ(getTable(Stock)->size(), i_stock_size);          \
    EXPECT_EQ(getTable(History)->size(), i_history_size);      \
  }

Json::Value TpccStoredStockLevelTest::doStockLevel(int w_id, int d_id, int threshold) {
  Json::Value data;
  data["W_ID"] = w_id;
  data["D_ID"] = d_id;
  data["threshold"] = threshold;

  return doStoredProcedure(data, "TPCC-StockLevel");
}

TEST_F(TpccStoredStockLevelTest, minimal) {
  //          (w_id, d_id, threshold);
  T_StockLevel(1, 1, 15);
}

TEST_F(TpccStoredStockLevelTest, DISABLED_StockLevel) {
  //          (w_id, d_id, threshold);
  T_StockLevel(1, 1, 15);
  T_StockLevel(1, 2, 20);
  T_StockLevel(2, 1, 11);
  T_StockLevel(2, 10, 20);
  T_StockLevel(1, 6, 16);
  T_StockLevel(2, 9, 18);
  T_StockLevel(1, 10, 10);
  T_StockLevel(2, 3, 15);
}

TEST_F(TpccStoredStockLevelTest, DISABLED_WrongDistrict) {
  //                       (w_id, d_id, threshold);
  EXPECT_THROW(doStockLevel(1, 0, 10), TpccError);
  EXPECT_THROW(doStockLevel(1, 11, 10), TpccError);
}

TEST_F(TpccStoredStockLevelTest, DISABLED_WrongThreshold) {
  //                       (w_id, d_id, threshold);
  EXPECT_THROW(doStockLevel(1, 1, 9), TpccError);
  EXPECT_THROW(doStockLevel(1, 1, 21), TpccError);
}
}
}  // namespace hyrise::access
