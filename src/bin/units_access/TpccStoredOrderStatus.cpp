// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TpccStoredProceduresTest.h"

namespace hyrise {
namespace access {

class TpccStoredOrderStatusTest : public TpccStoredProceduresTest {

 protected:
  Json::Value doOrderStatus(int w_id, int d_id, int c_id, const std::string& c_last);
};

// TEST_F(TpccStoredOrderStatusTest, OrderStatusTest) {
Json::Value TpccStoredOrderStatusTest::doOrderStatus(int w_id, int d_id, int c_id, const std::string& c_last) {
  bool selectById = (c_last.size() == 0);

  Json::Value data;
  data["W_ID"] = w_id;
  data["D_ID"] = d_id;
  if (selectById)
    data["C_ID"] = c_id;
  else
    data["C_LAST"] = c_last;

  return doStoredProcedure(data, "TPCC-OrderStatus");
}



#define T_OrderStatus_check_values(w_id, d_id, c_id, c_last)              \
  EXPECT_EQ(w_id, getValuei(response, "W_ID"));                           \
  EXPECT_EQ(d_id, getValuei(response, "D_ID"));                           \
  /*customer stuff*/                                                      \
  const auto c_nbr = toString(c_id);                                      \
  EXPECT_EQ("CFName" + c_nbr, getValues(response, "C_FIRST"));            \
  EXPECT_EQ("CMName" + c_nbr, getValues(response, "C_MIDDLE"));           \
  EXPECT_EQ(c_last, getValues(response, "C_LAST"));                       \
  EXPECT_FLOAT_EQ(-10.0f, getValuef(response, "C_BALANCE"));              \
  /*order stuff*/                                                         \
  EXPECT_EQ(c_id, getValuei(response, "O_ID"));                           \
  EXPECT_TRUE(response.isMember("O_ENTRY_D")); /*TODO maybe check value*/ \
  EXPECT_EQ(c_id, getValuei(response, "O_CARRIER_ID"));                   \
                                                                          \
  const Json::Value& ols = response["order line"];                        \
  for (size_t i = 0; i < ols.size(); ++i) {                               \
    const Json::Value& cur = ols[(int)i];                                 \
    EXPECT_EQ(std::min(c_id, 2), getValuei(cur, "OL_SUPPLY_W_ID"));       \
    const int i_id = getValuei(cur, "OL_I_ID");                           \
    EXPECT_EQ(2, getValuei(cur, "OL_QUANTITY"));                          \
    EXPECT_FLOAT_EQ(2 * 1.01 * i_id, getValuef(cur, "OL_AMOUNT"));        \
    getValues(cur, "OL_DELIVERY_D"); /*TODO check for value*/             \
  }                                                                       \
                                                                          \
  EXPECT_EQ(getTable(Customer)->size(), i_customer_size);                 \
  EXPECT_EQ(getTable(Orders)->size(), i_orders_size);                     \
  EXPECT_EQ(getTable(OrderLine)->size(), i_orderLine_size);               \
  EXPECT_EQ(getTable(Warehouse)->size(), i_warehouse_size);               \
  EXPECT_EQ(getTable(NewOrder)->size(), i_newOrder_size);                 \
  EXPECT_EQ(getTable(District)->size(), i_district_size);                 \
  EXPECT_EQ(getTable(Item)->size(), i_item_size);                         \
  EXPECT_EQ(getTable(Stock)->size(), i_stock_size);                       \
  EXPECT_EQ(getTable(History)->size(), i_history_size);



#define T_OrderStatusId(w_id, d_id, c_id)                              \
  {                                                                    \
    const auto response = doOrderStatus(w_id, d_id, c_id, "");         \
                                                                       \
    const std::string c_last = "CLName" + toString(std::min(c_id, 2)); \
    T_OrderStatus_check_values(w_id, d_id, c_id, c_last)               \
  }

#define T_OrderStatusCLast(w_id, d_id, c_last)                  \
  {                                                             \
    const auto response = doOrderStatus(w_id, d_id, 0, c_last); \
                                                                \
    const std::string start = "CLName";                         \
    const std::string clast = c_last;                           \
    const int c_id = stol(clast.substr(start.size()));          \
    T_OrderStatus_check_values(w_id, d_id, c_id, c_last)        \
  }

TEST_F(TpccStoredOrderStatusTest, minimal) {
  //             (w_id, d_id, c_id);
  T_OrderStatusId(1, 1, 5);

  //                (w_id, d_id, c_last         );
  T_OrderStatusCLast(2, 3, "CLName2");
}

TEST_F(TpccStoredOrderStatusTest, DISABLED_OrderStatus) {
  //             (w_id, d_id, c_id);
  T_OrderStatusId(1, 1, 5);
  T_OrderStatusId(2, 1, 2);
  T_OrderStatusId(1, 3, 1);
  T_OrderStatusId(2, 10, 2);
  T_OrderStatusId(2, 8, 1);
  //                (w_id, d_id, c_last         );
  T_OrderStatusCLast(2, 3, "CLName2");
  T_OrderStatusCLast(1, 5, "CLName1");
  T_OrderStatusCLast(1, 10, "CLName1");
  T_OrderStatusCLast(2, 1, "CLName1");
}

TEST_F(TpccStoredOrderStatusTest, DISABLED_wrongD_ID) {
  //                        (w_id, d_id, c_id, c_last         );
  EXPECT_THROW(doOrderStatus(1, 0, 1, ""), TpccError);
  EXPECT_THROW(doOrderStatus(1, 11, 1, ""), TpccError);
  EXPECT_THROW(doOrderStatus(1, 0, 1, "CLName1"), TpccError);
  EXPECT_THROW(doOrderStatus(1, 11, 1, "CLName2"), TpccError);
}
}
}  // namespace hyrise::access
