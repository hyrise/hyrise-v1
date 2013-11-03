// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/shortcuts.h"
#include "testing/test.h"

#include "helper.h"
#include <fstream>
#include <json.h>

#include <io/ResourceManager.h>

namespace hyrise {
namespace access {

namespace {
  typedef struct item_info_t { int id; int w_id; int quantity; } ItemInfo;
  typedef std::vector<ItemInfo> item_info_list_t;
} // namespace

class TpccError : public std::runtime_error {
 public:
  TpccError(const std::string what) : std::runtime_error(what) {}
};

class TpccStoredProceduresTest : public AccessTest {
public:
  void SetUp() {
    loadTables();

    i_customer_size  = getTable(Customer)->size();
    i_orders_size    = getTable(Orders)->size();
    i_orderLine_size = getTable(OrderLine)->size();
    i_warehouse_size = getTable(Warehouse)->size();
    i_newOrder_size  = getTable(NewOrder)->size();
    i_district_size  = getTable(District)->size();
    i_item_size      = getTable(Item)->size();
    i_stock_size     = getTable(Stock)->size();
    i_history_size   = getTable(History)->size();
  }

  void TearDown() {
    io::ResourceManager::getInstance().clear();
  }

protected:
  void loadTables() {
    executeAndWait(loadFromFile("test/tpcc/load_tpcc_tables.json"));
  }

  Json::Value doDelivery(int w_id, int d_id, int o_carrier_id);
  Json::Value doNewOrder(int w_id, int d_id, int c_id, int o_carrier_id, std::string ol_dist_info, item_info_list_t items);
  Json::Value doNewOrderR(int w_id, int d_id, int c_id, int o_carrier_id, std::string ol_dist_info, item_info_list_t items);
  Json::Value doOrderStatus(int w_id, int d_id, int c_id, std::string c_last);
  Json::Value doPayment(int w_id, int d_id, int c_id, std::string c_last, int c_w_id, int c_d_id, float h_amount, bool bc_customer);
  Json::Value doStockLevel(int w_id, int d_id, int threshold);

  size_t i_customer_size, i_orders_size, i_orderLine_size, i_warehouse_size, i_newOrder_size,
         i_district_size, i_item_size, i_stock_size, i_history_size;


  enum TpccTable { Customer, Orders, OrderLine, Warehouse, NewOrder, District, Item, Stock, History };

  static storage::c_atable_ptr_t getTable(const TpccTable table, tx::transaction_id_t tid = hyrise::tx::UNKNOWN) {
    std::string tableName;
    std::string fileName;
    switch (table) {
      case Customer:  tableName = "CUSTOMER"; break;
      case Orders:    tableName = "ORDERS"; break;
      case OrderLine: tableName = "ORDER_LINE"; break;
      case Warehouse: tableName = "WAREHOUSE"; break;
      case NewOrder:  tableName = "NEW_ORDER"; break;
      case District:  tableName = "DISTRICT"; break;
      case Item:      tableName = "ITEM"; break;
      case Stock:     tableName = "STOCK"; break;
      case History:   tableName = "HISTORY"; break;
    }
    if (tid == hyrise::tx::UNKNOWN)
      tid = getNewTXContext().tid;

    return executeAndWait("{\"operators\": {\"load\": {\"type\": \"GetTable\", \"name\": \"" + tableName + "\"}" +
                          ", \"validate\": {\"type\": \"ValidatePositions\"}}, \"edges\": [[\"load\", \"validate\"]]}", 1, nullptr, tid);
  }
};

TEST_F(TpccStoredProceduresTest, LoadTables) {
  auto customer = getTable(Customer);
  EXPECT_LT(0, customer->size()) << "invalid customer table";

  auto orders = getTable(Orders);
  EXPECT_LT(0, orders->size()) << "invalid orders table";
  
  auto orderLine = getTable(OrderLine);
  EXPECT_LT(0, orderLine->size()) << "invalid order_line table";
  
  auto warehouse = getTable(Warehouse);
  EXPECT_LT(0, warehouse->size()) << "invalid warehouse table";
  
  auto newOrder = getTable(NewOrder);
  EXPECT_LT(0, newOrder->size()) << "invalid new_order table";
  
  auto district = getTable(District);
  EXPECT_LT(0, district->size()) << "invalid district table";
  
  auto item = getTable(Item);
  EXPECT_LT(0, item->size()) << "invalid item table";
  
  auto stock = getTable(Stock);
  EXPECT_LT(0, stock->size()) << "invalid stock table";
  
  auto history = getTable(History);
  EXPECT_LT(0, history->size()) << "invalid history table";
}

//TEST_F(TpccStoredProceduresTest, DeliveryTest) {
Json::Value TpccStoredProceduresTest::doDelivery(int w_id, int d_id, int o_carrier_id) {
  Json::Value data;
  data["W_ID"] = w_id;
  data["D_ID"] = d_id;

  Json::StyledWriter writer;
  const auto s = executeStoredProcedureAndWait("TPCC-Delivery", writer.write(data));

  //simple size checks on tables:
  EXPECT_EQ(i_orders_size, getTable(Orders)->size()) << "number of rows in ORDERS should not change";
  EXPECT_EQ(i_orderLine_size, getTable(OrderLine)->size()) << "number of rows in ORDER-LINE should not change";
  EXPECT_EQ(i_customer_size, getTable(Customer)->size()) << "number of rows in CUSTOMER should not change";
  
  //EXPECT_EQ(i_newOrder_size, getTable(NewOrder)->size() + 1) << "number of rows in New-Order should be exactly one less";
  Json::Reader reader;
  Json::Value response;
  if (!reader.parse(s, response))
    throw TpccError(s);
  return response;
}

namespace {
  Json::Value newOrderData(int w_id, int d_id, int c_id, int o_carrier_id, std::string ol_dist_info, item_info_list_t items) {
    Json::Value data;
    data["W_ID"] = w_id;
    data["D_ID"] = d_id;
    data["C_ID"] = c_id;
    data["O_CARRIER_ID"] = o_carrier_id;
    data["OL_DIST_INFO"] = ol_dist_info;

    Json::Value itemData(Json::arrayValue);
    for (size_t i = 0; i < items.size(); ++i) {
       Json::Value item;
       item["I_ID"] = items.at(i).id;
       item["I_W_ID"] = items.at(i).w_id;
       item["quantity"] = items.at(i).quantity;
       itemData.append(item);
    }
    data["items"] = itemData;

    return data;
  }
} // namespace

//TEST_F(TpccStoredProceduresTest, NewOrderTest) {
Json::Value TpccStoredProceduresTest::doNewOrder(int w_id, int d_id, int c_id, int o_carrier_id, std::string ol_dist_info,
                                                 item_info_list_t items) {
  auto data = newOrderData(w_id, d_id, c_id, o_carrier_id, ol_dist_info, items);

  Json::StyledWriter writer;
  const auto s = executeStoredProcedureAndWait("TPCC-NewOrder", writer.write(data));
  Json::Reader reader;
  Json::Value response;
  if (!reader.parse(s, response))
    throw TpccError(s);

  //simple size checks on tables:
  EXPECT_EQ(i_stock_size, getTable(Stock)->size())
           << "number of rows in STOCK should not change";
 
  EXPECT_EQ(i_newOrder_size, getTable(NewOrder)->size() - 1)
           << "number of rows in NEW-ORDER should be exactly one more";
  EXPECT_EQ(i_orders_size, getTable(Orders)->size() - 1)
           << "number of rows in ORDERS should be exactly one more";
  EXPECT_EQ(i_orderLine_size, getTable(OrderLine)->size() - items.size())
           << "number of rows in ORDER-LINES should be exactly " << items.size() << " more";

  return response;
}

//TEST_F(TpccStoredProceduresTest, NewOrderTest) {
Json::Value TpccStoredProceduresTest::doNewOrderR(int w_id, int d_id, int c_id, int o_carrier_id, const std::string ol_dist_info,
                                                  const item_info_list_t items) {
  auto data = newOrderData(w_id, d_id, c_id, o_carrier_id, ol_dist_info, items);

  Json::StyledWriter writer;
  const auto s = executeStoredProcedureAndWait("TPCC-NewOrder", writer.write(data));
  Json::Reader reader;
  Json::Value response;
  if (!reader.parse(s, response))
    throw TpccError(s);

  EXPECT_EQ(i_stock_size, getTable(Stock)->size()) << "number of rows in STOCK should not change";
  EXPECT_EQ(i_newOrder_size, getTable(NewOrder)->size()) << "number of rows in NEW-ORDER should not change";
  EXPECT_EQ(i_orders_size, getTable(Orders)->size()) << "number of rows in ORDERS should not change";
  EXPECT_EQ(i_orderLine_size, getTable(OrderLine)->size()) << "number of rows in ORDER-LINES should not change";
  
  return response;
}

//TEST_F(TpccStoredProceduresTest, OrderStatusTest) {
Json::Value TpccStoredProceduresTest::doOrderStatus(int w_id, int d_id, int c_id, std::string c_last) {
  bool selectById = (c_last.size() == 0);

  Json::Value data;
  data["W_ID"] = w_id;
  data["D_ID"] = d_id;
  if (selectById)
    data["C_ID"] = d_id;
  else
    data["C_LAST"] = c_last;

  Json::StyledWriter writer;

  const auto s = executeStoredProcedureAndWait("TPCC-OrderStatus", writer.write(data));
  Json::Reader reader;
  Json::Value response;
  if (!reader.parse(s, response))
    throw TpccError(s);

  EXPECT_EQ(i_customer_size, getTable(Customer)->size()) << "number of rows in CUSTOMER should not change";
  EXPECT_EQ(i_warehouse_size, getTable(Warehouse)->size()) << "number of rows in WAREHOUSE should not change";
  EXPECT_EQ(i_district_size, getTable(District)->size()) << "number of rows in DISTRICT should not change";

  return response;
}

Json::Value TpccStoredProceduresTest::doPayment(int w_id, int d_id, int c_id, std::string c_last, int c_w_id,
                                                int c_d_id, float h_amount, bool bc_customer) {
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

  Json::StyledWriter writer;
    
  const auto s = executeStoredProcedureAndWait("TPCC-Payment", writer.write(data));
  Json::Reader reader;
  Json::Value response;
  if (!reader.parse(s, response))
    throw TpccError(s);

  return response;
}

Json::Value TpccStoredProceduresTest::doStockLevel(int w_id, int d_id, int threshold) {
  Json::Value data;
  data["W_ID"] = w_id;
  data["D_ID"] = d_id;
  data["threshold"] = threshold;

  Json::StyledWriter writer;
    
  const auto s = executeStoredProcedureAndWait("TPCC-StockLevel", writer.write(data));
  Json::Reader reader;
  Json::Value response;
  if (!reader.parse(s, response))
    throw TpccError(s);

  return response;
}




//===========================Delivery Tests

TEST_F(TpccStoredProceduresTest, Delivery_W1D1) {
  //                              (w_id, d_id, o_carrier_id);
  const auto response = doDelivery(1   , 1   , 1           );
}

TEST_F(TpccStoredProceduresTest, Delivery_WrongCarrierId) {
  //                     (w_id, d_id, o_carrier_id);
  EXPECT_THROW(doDelivery(1   , 1   , 0           ), TpccError);
  EXPECT_THROW(doDelivery(1   , 1   , 11          ), TpccError);
}

TEST_F(TpccStoredProceduresTest, Delivery_WrongDistrictId) {
  //                     (w_id, d_id, o_carrier_id);
  EXPECT_THROW(doDelivery(1   , 0   , 1           ), TpccError);
  EXPECT_THROW(doDelivery(1   , 11  , 1           ), TpccError);
}

//===========================New Order Tests

TEST_F(TpccStoredProceduresTest, NewOrder_tooFewItems) {
  //                     (w_id, d_id, c_id, o_carrier_id, ol_dist_info, {{i_id, i_w_id, quantity}});
  EXPECT_THROW(doNewOrder(1   , 1   , 1   , 1           , "info"      , {{1   , 1     , 1       }}), TpccError);
}

TEST_F(TpccStoredProceduresTest, NewOrder_wrongQuantity11) {
  //                     (w_id, d_id, c_id, o_carrier_id, ol_dist_info, {{i_id, i_w_id, quantity}});
  EXPECT_THROW(doNewOrder(1   , 1   , 1   , 1           , "info"      , {{1   , 1     , 1       },
                                                                         {2   , 1     , 1       },
                                                                         {3   , 1     , 1       },
                                                                         {4   , 1     , 1       },
                                                                         {5   , 1     , 11      }, }), TpccError);
}

TEST_F(TpccStoredProceduresTest, NewOrder_wrongQuantity0) {
  //                     (w_id, d_id, c_id, o_carrier_id, ol_dist_info, {{i_id, i_w_id, quantity}});
  EXPECT_THROW(doNewOrder(1   , 1   , 1   , 1           , "info"      , {{1   , 1     , 1       },
                                                                         {2   , 1     , 1       },
                                                                         {3   , 1     , 1       },
                                                                         {4   , 1     , 1       },
                                                                         {5   , 1     , 0       }, }), TpccError);
}

TEST_F(TpccStoredProceduresTest, NewOrder_twiceTheSameItem) {
  //                     (w_id, d_id, c_id, o_carrier_id, ol_dist_info, {{i_id, i_w_id, quantity}});
  //TODO this fails but because stock info for i_id=4 and w_id=1 is missing (which actually isn't)
  /*EXPECT_THROW(*/doNewOrder(1   , 1   , 1   , 1           , "info"      , {{1   , 1     , 1       },
                                                                         {2   , 1     , 1       },
                                                                         {3   , 1     , 1       },
                                                                         {4   , 1     , 1       },
                                                                         {4   , 1     , 1       }, });//, TpccError);
}

TEST_F(TpccStoredProceduresTest, NewOrder_W1D1C1) {
  //                              (w_id, d_id, c_id, o_carrier_id, ol_dist_info, {{i_id, i_w_id, quantity}});
  const auto response = doNewOrder(1   , 1   , 1   , 1           , "info"      , {{1   , 1     , 1       },
                                                                                  {2   , 1     , 2       },
                                                                                  {3   , 1     , 3       },
                                                                                  {4   , 1     , 4       },
                                                                                  {5   , 1     , 5       }});
}

TEST_F(TpccStoredProceduresTest, NewOrder_W1D1C1rollback) {
  //                              (w_id, d_id, c_id, o_carrier_id, ol_dist_info, {{i_id, i_w_id, quantity}});
  const auto response = doNewOrder(1   , 1   , 1   , 1           , "info"      , {{1   , 1     , 10      },
                                                                                  {2   , 1     , 10      },
                                                                                  {3   , 1     , 10      },
                                                                                  {4   , 1     , 10      },
                                                                                  {5   , 1     , 10      },
                                                                                  {6   , 1     , 10      },
                                                                                  {7   , 1     , 10      },
                                                                                  {8   , 1     , 10      },
                                                                                  {9   , 1     , 10      },
                                                                                  {10  , 1     , 10      },
                                                                                  {11  , 1     , 10      },
                                                                                  {12  , 1     , 10      },
                                                                                  {13  , 1     , 10      },
                                                                                  {14  , 1     , 10      },
                                                                                  {15  , 1     , 10      }});
}

//===========================Order Status Tests

TEST_F(TpccStoredProceduresTest, OrderStatus_W1D1C1) {
  //                                 (w_id, d_id, c_id, c_last         );
  const auto response = doOrderStatus(1   , 1   , 1   , ""             );
}

TEST_F(TpccStoredProceduresTest, OrderStatus_W1D1Name1) {
  //                                 (w_id, d_id, c_id, c_last         );
  const auto response = doOrderStatus(1   , 1   , 1   , "CLName1"      );
}

//============================Payment Tests

TEST_F(TpccStoredProceduresTest, Payment_W1D1C1localBC) {
  //                             (w_id, d_id, c_id, c_last        , c_w_id, c_d_id, h_amount, bc_customer);
  const auto response = doPayment(1   , 1   , 1   , ""            , 1     , 1     , 300.0f  , true       );
}

TEST_F(TpccStoredProceduresTest, Payment_W1D1Name2localGC) {
  //                             (w_id, d_id, c_id, c_last        , c_w_id, c_d_id, h_amount, bc_customer);
  const auto response = doPayment(1   , 1   , 1   , "CLName2"     , 1     , 1     , 150.0f  , false      );
}

//============================Stock Level Tests

void assureFieldExists(const Json::Value& data, const std::string& name) {
  if (!data.isMember(name))
    throw std::runtime_error("\"" + name + "\" should be set but is not");
}

int getValuei(const Json::Value& data, std::string name) {
  assureFieldExists(data, name);
  return data[name].asInt();
}

float getValuef(const Json::Value& data, std::string name) {
  assureFieldExists(data, name);
  return data[name].asFloat();
}

TEST_F(TpccStoredProceduresTest, StockLevel_W1D1T90) {
  //                                (w_id, d_id, threshold);
  const auto response = doStockLevel(1   , 1   , 90       );

  ASSERT_EQ(1, getValuei(response, "W_ID"));
  ASSERT_EQ(1, getValuei(response, "D_ID"));
  ASSERT_EQ(90, getValuei(response, "threshold"));
  ASSERT_EQ(5, getValuei(response, "low_stock"));
}

TEST_F(TpccStoredProceduresTest, StockLevel_W1D1T50) {
  //                                (w_id, d_id, threshold);
  const auto response = doStockLevel(1   , 1   , 50       );

  ASSERT_EQ(1, getValuei(response, "W_ID"));
  ASSERT_EQ(1, getValuei(response, "D_ID"));
  ASSERT_EQ(50, getValuei(response, "threshold"));
  ASSERT_EQ(0, getValuei(response, "low_stock"));
}

} } // namespace hyrise::access

