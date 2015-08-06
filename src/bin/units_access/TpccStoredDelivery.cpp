// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TpccStoredProceduresTest.h"

namespace hyrise {
namespace access {

class TpccStoredDeliveryTest : public TpccStoredProceduresTest {

 protected:
  Json::Value doDelivery(int w_id, int d_id, int o_carrier_id);
};

Json::Value TpccStoredDeliveryTest::doDelivery(int w_id, int d_id, int o_carrier_id) {
  Json::Value data;
  data["W_ID"] = w_id;
  data["D_ID"] = d_id;
  data["O_CARRIER_ID"] = o_carrier_id;

  return doStoredProcedure(data, "TPCC-Delivery");
}



#define T_Delivery(w_id, d_id, o_carrier_id)                                                                     \
  {                                                                                                              \
    const auto response = doDelivery(w_id, d_id, o_carrier_id);                                                  \
                                                                                                                 \
    EXPECT_EQ(w_id, getValuei(response, "W_ID"));                                                                \
    EXPECT_EQ(o_carrier_id, getValuei(response, "O_CARRIER_ID"));                                                \
                                                                                                                 \
    EXPECT_EQ(getTable(Customer)->size(), i_customer_size);                                                      \
    EXPECT_EQ(getTable(Orders)->size(), i_orders_size);                                                          \
    EXPECT_EQ(getTable(OrderLine)->size(), i_orderLine_size);                                                    \
    EXPECT_EQ(getTable(Warehouse)->size(), i_warehouse_size);                                                    \
    /*todo hard to tell how NewOrders size will change) EXPECT_EQ(getTable(NewOrder)->size() , i_newOrder_size - \
     * 10);*/                                                                                                    \
    EXPECT_EQ(getTable(District)->size(), i_district_size);                                                      \
    EXPECT_EQ(getTable(Item)->size(), i_item_size);                                                              \
    EXPECT_EQ(getTable(Stock)->size(), i_stock_size);                                                            \
    EXPECT_EQ(getTable(History)->size(), i_history_size);                                                        \
  }

TEST_F(TpccStoredDeliveryTest, minimal) {
  //        (w_id, d_id, o_carrier_id);
  T_Delivery(2, 1, 5);
}

TEST_F(TpccStoredDeliveryTest, DISABLED_Delivery) {
  //        (w_id, d_id, o_carrier_id);
  T_Delivery(1, 1, 1);
  T_Delivery(2, 3, 5);
  T_Delivery(2, 4, 10);
  T_Delivery(1, 1, 4);
  T_Delivery(2, 5, 2);
  T_Delivery(1, 1, 2);
  T_Delivery(2, 3, 9);
  T_Delivery(1, 6, 1);
  T_Delivery(1, 10, 8);
  T_Delivery(1, 10, 8);
}

TEST_F(TpccStoredDeliveryTest, DISABLED_WrongCARRIER_ID) {
  //                     (w_id, d_id, o_carrier_id);
  EXPECT_THROW(doDelivery(1, 1, 0), TpccError);
  EXPECT_THROW(doDelivery(1, 1, 11), TpccError);
}
}
}  // namespace hyrise::access
