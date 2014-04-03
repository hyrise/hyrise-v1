// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TpccStoredProceduresTest.h"

namespace hyrise {
namespace access {

class TpccStoredPaymentTest : public TpccStoredProceduresTest {

 protected:
  Json::Value
      doPayment(int w_id, int d_id, int c_id, const std::string& c_last, int c_w_id, int c_d_id, float h_amount);
};

Json::Value TpccStoredPaymentTest::doPayment(int w_id,
                                             int d_id,
                                             int c_id,
                                             const std::string& c_last,
                                             int c_w_id,
                                             int c_d_id,
                                             float h_amount) {
  const bool selectById = (c_last.size() == 0);

  Json::Value data;
  data["W_ID"] = w_id;
  data["D_ID"] = d_id;

  if (selectById)
    data["C_ID"] = c_id;
  else
    data["C_LAST"] = c_last;

  data["C_W_ID"] = c_w_id;
  data["C_D_ID"] = c_d_id;
  data["H_AMOUNT"] = h_amount;

  return doStoredProcedure(data, "TPCC-Payment");
}

#define T_DISABLED_check_values(w_id, d_id, c_id, c_last, c_w_id, c_d_id, h_amount) \
  EXPECT_EQ(w_id, getValuei(response, "W_ID"));                                     \
  EXPECT_EQ(d_id, getValuei(response, "D_ID"));                                     \
  EXPECT_EQ(c_id, getValuei(response, "C_ID"));                                     \
  EXPECT_EQ(c_w_id, getValuei(response, "C_W_ID"));                                 \
  EXPECT_EQ(c_d_id, getValuei(response, "C_D_ID"));                                 \
  EXPECT_EQ(h_amount, getValuef(response, "H_AMOUNT"));                             \
  /*warehouse stuff*/                                                               \
  const auto w_nbr = toString(w_id);                                                \
  EXPECT_EQ("WStreet1-" + w_nbr, getValues(response, "W_STREET_1"));                \
  EXPECT_EQ("WStreet2-" + w_nbr, getValues(response, "W_STREET_2"));                \
  EXPECT_EQ("WCity" + w_nbr, getValues(response, "W_CITY"));                        \
  EXPECT_EQ("WState" + w_nbr, getValues(response, "W_STATE"));                      \
  EXPECT_EQ("WZip" + w_nbr, getValues(response, "W_ZIP"));                          \
  /*district stuff*/                                                                \
  const auto d_nbr = toString((w_id - 1) * 10 + d_id);                              \
  EXPECT_EQ("DStreet1-" + d_nbr, getValues(response, "D_STREET_1"));                \
  EXPECT_EQ("DStreet2-" + d_nbr, getValues(response, "D_STREET_2"));                \
  EXPECT_EQ("D_City" + d_nbr, getValues(response, "D_CITY"));                       \
  EXPECT_EQ("D_State" + d_nbr, getValues(response, "D_STATE"));                     \
  EXPECT_EQ("D_Zip" + d_nbr, getValues(response, "D_ZIP"));                         \
  /*customer stuff*/                                                                \
  const auto c_nbr = toString(c_id);                                                \
  EXPECT_EQ("CFName" + c_nbr, getValues(response, "C_FIRST"));                      \
  EXPECT_EQ("CMName" + c_nbr, getValues(response, "C_MIDDLE"));                     \
  EXPECT_EQ("CLName" + toString(std::min(c_id, 2)), getValues(response, "C_LAST")); \
  EXPECT_EQ("CStreet1-" + c_nbr, getValues(response, "C_STREET_1"));                \
  EXPECT_EQ("CStreet2-" + c_nbr, getValues(response, "C_STREET_2"));                \
  EXPECT_EQ("CCity" + c_nbr, getValues(response, "C_CITY"));                        \
  EXPECT_EQ("CState" + c_nbr, getValues(response, "C_STATE"));                      \
  EXPECT_EQ("CZip" + c_nbr, getValues(response, "C_ZIP"));                          \
  EXPECT_EQ("CPhone" + c_nbr, getValues(response, "C_PHONE"));                      \
  EXPECT_TRUE(response.isMember("C_SINCE")); /*TODO maybe check value*/             \
  EXPECT_TRUE(response.isMember("C_CREDIT")); /*TODO maybe check value*/            \
  EXPECT_TRUE(response.isMember("C_CREDIT_LIM")); /*TODO maybe check value*/        \
  EXPECT_FLOAT_EQ(0.01 * c_id, getValuef(response, "C_DISCOUNT"));                  \
  EXPECT_TRUE(response.isMember("C_BALANCE")); /*TODO maybe check value*/           \
  EXPECT_TRUE(response.isMember("C_DATA")); /*TODO maybe check value*/

#define T_PaymentId(w_id, d_id, c_id, c_w_id, c_d_id, h_amount)                      \
  {                                                                                  \
    const auto response = doPayment(w_id, d_id, c_id, "", c_w_id, c_d_id, h_amount); \
                                                                                     \
    const std::string c_last = "CLName" + toString(std::min(c_id, 2));               \
    T_DISABLED_check_values(w_id, d_id, c_id, c_last, c_w_id, c_d_id, h_amount)      \
        EXPECT_EQ(getTable(Customer)->size(), i_customer_size);                      \
    EXPECT_EQ(getTable(Orders)->size(), i_orders_size);                              \
    EXPECT_EQ(getTable(OrderLine)->size(), i_orderLine_size);                        \
    EXPECT_EQ(getTable(Warehouse)->size(), i_warehouse_size);                        \
    EXPECT_EQ(getTable(NewOrder)->size(), i_newOrder_size);                          \
    EXPECT_EQ(getTable(District)->size(), i_district_size);                          \
    EXPECT_EQ(getTable(Item)->size(), i_item_size);                                  \
    EXPECT_EQ(getTable(Stock)->size(), i_stock_size);                                \
    EXPECT_EQ(getTable(History)->size(), i_history_size + 1);                        \
    i_history_size += 1;                                                             \
  }

#define T_PaymentCLast(w_id, d_id, c_last, c_w_id, c_d_id, h_amount, c_id)            \
  {                                                                                   \
    const auto response = doPayment(w_id, d_id, 0, c_last, c_w_id, c_d_id, h_amount); \
                                                                                      \
    T_DISABLED_check_values(w_id, d_id, c_id, c_last, c_w_id, c_d_id, h_amount)       \
        EXPECT_EQ(getTable(Customer)->size(), i_customer_size);                       \
    EXPECT_EQ(getTable(Orders)->size(), i_orders_size);                               \
    EXPECT_EQ(getTable(OrderLine)->size(), i_orderLine_size);                         \
    EXPECT_EQ(getTable(Warehouse)->size(), i_warehouse_size);                         \
    EXPECT_EQ(getTable(NewOrder)->size(), i_newOrder_size);                           \
    EXPECT_EQ(getTable(District)->size(), i_district_size);                           \
    EXPECT_EQ(getTable(Item)->size(), i_item_size);                                   \
    EXPECT_EQ(getTable(Stock)->size(), i_stock_size);                                 \
    EXPECT_EQ(getTable(History)->size(), i_history_size + 1);                         \
    i_history_size += 1;                                                              \
  }

TEST_F(TpccStoredPaymentTest, minimal) {
  //         (w_id, d_id, c_id, c_w_id, c_d_id, h_amount);
  T_PaymentId(1, 1, 3, 1, 1, 300.0f);
  //            (w_id, d_id, c_last        , c_w_id, c_d_id, h_amount);
  T_PaymentCLast(1, 1, "CLName2", 1, 1, 123.0f, 3);
}

TEST_F(TpccStoredPaymentTest, DISABLED_Payment) {
  //         (w_id, d_id, c_id, c_w_id, c_d_id, h_amount);
  T_PaymentId(1, 1, 3, 1, 1, 300.0f);
  T_PaymentId(1, 1, 3, 1, 1, 1000.0f);
  T_PaymentId(1, 2, 1, 2, 1, 900.0f);
  T_PaymentId(2, 1, 2, 1, 3, 550.0f);
  T_PaymentId(1, 3, 1, 1, 10, 111.11f);
  T_PaymentId(2, 1, 2, 1, 3, 777.0f);
  T_PaymentId(2, 5, 2, 2, 5, 123.45f);
  T_PaymentId(2, 5, 2, 2, 5, 123.45f);

  //            (w_id, d_id, c_last        , c_w_id, c_d_id, h_amount);
  T_PaymentCLast(1, 1, "CLName2", 1, 1, 123.0f, 3);
  // T_PaymentCLast(2   , 9   , "CLName2"     , 1     , 1     , 234.0f  );
  // T_PaymentCLast(1   , 1   , "CLName1"     , 1     , 1     , 355.0f  );
  // T_PaymentCLast(2   , 7   , "CLName1"     , 2     , 10    , 456.0f  );
  // T_PaymentCLast(2   , 7   , "CLName2"     , 1     , 2     , 567.0f  );
  // T_PaymentCLast(1   , 7   , "CLName2"     , 1     , 10    , 687.0f  );
  // T_PaymentCLast(1   , 1   , "CLName2"     , 2     , 10    , 798.0f  );
  // T_PaymentCLast(2   , 3   , "CLName2"     , 1     , 2     , 809.0f  );
}

TEST_F(TpccStoredPaymentTest, DISABLED_wrongAmount) {
  //                    (w_id, d_id, c_id, c_last        , c_w_id, c_d_id, h_amount);
  EXPECT_THROW(doPayment(1, 1, 1, "", 1, 1, 0.99f), TpccError);
  EXPECT_THROW(doPayment(1, 1, 1, "", 1, 1, 5000.01f), TpccError);
}

TEST_F(TpccStoredPaymentTest, DISABLED_wrongD_ID) {
  //                    (w_id, d_id, c_id, c_last        , c_w_id, c_d_id, h_amount);
  EXPECT_THROW(doPayment(1, 0, 1, "", 1, 1, 100.0f), TpccError);
  EXPECT_THROW(doPayment(1, 11, 1, "", 1, 1, 100.0f), TpccError);
  EXPECT_THROW(doPayment(1, 1, 1, "", 1, 0, 100.0f), TpccError);
  EXPECT_THROW(doPayment(1, 1, 1, "", 1, 11, 100.0f), TpccError);
}
}
}  // namespace hyrise::access
