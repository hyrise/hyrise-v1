// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/shortcuts.h"
#include "testing/test.h"

#include "helper.h"
#include <fstream>
#include <json.h>

#include <io/ResourceManager.h>

namespace {
  const std::string tpccQueryPath = "test/json/tpcc";
  const std::string niceDate = "2013-09-20-02-16-31";
}

namespace hyrise {
namespace access {

class TpccStoredProceduresTest : public AccessTest {
public:
  TpccStoredProceduresTest() {
    _deliveryMap["deleteNewOrder"]  = tpccQueryPath + "/Delivery-deleteNewOrder.json";
    _deliveryMap["getCId"]          = tpccQueryPath + "/Delivery-getCId.json";
    _deliveryMap["getNewOrder"]     = tpccQueryPath + "/Delivery-getNewOrder.json";
    _deliveryMap["sumOLAmount"]     = tpccQueryPath + "/Delivery-sumOLAmount.json";
    _deliveryMap["updateCustomer"]  = tpccQueryPath + "/Delivery-updateCustomer.json";
    _deliveryMap["updateOrderLine"] = tpccQueryPath + "/Delivery-updateOrderLine.json";
    _deliveryMap["updateOrders"]    = tpccQueryPath + "/Delivery-updateOrders.json";

    _newOrderMap["createNewOrder"]       = tpccQueryPath + "/NewOrder-createNewOrder.json";
    _newOrderMap["createOrderLine"]      = tpccQueryPath + "/NewOrder-createOrderLine.json";
    _newOrderMap["createOrder"]          = tpccQueryPath + "/NewOrder-createOrder.json";
    _newOrderMap["getCustomer"]          = tpccQueryPath + "/NewOrder-getCustomer.json";
    _newOrderMap["getDistrict"]          = tpccQueryPath + "/NewOrder-getDistrict.json";
    _newOrderMap["getItemInfo"]          = tpccQueryPath + "/NewOrder-getItemInfo.json";
    _newOrderMap["getStockInfo"]         = tpccQueryPath + "/NewOrder-getStockInfo.json";
    _newOrderMap["getWarehouseTaxRate"]  = tpccQueryPath + "/NewOrder-getWarehouseTaxRate.json";
    _newOrderMap["incrementNextOrderId"] = tpccQueryPath + "/NewOrder-incrementNextOrderId.json";
    _newOrderMap["updateStock"]          = tpccQueryPath + "/NewOrder-updateStock.json";

    _orderStatusMap["getCustomerByCId"]       = tpccQueryPath + "/OrderStatus-getCustomerByCId.json";
    _orderStatusMap["getCustomersByLastName"] = tpccQueryPath + "/OrderStatus-getCustomersByLastName.json";
    _orderStatusMap["getLastOrder"]           = tpccQueryPath + "/OrderStatus-getLastOrder.json";
    _orderStatusMap["getOrderLines"]          = tpccQueryPath + "/OrderStatus-getOrderLines.json";

    _paymentMap["getCustomerByCId"]       = tpccQueryPath + "/Payment-getCustomerByCId.json";
    _paymentMap["getCustomersByLastName"] = tpccQueryPath + "/Payment-getCustomersByLastName.json";
    _paymentMap["getDistrict"]            = tpccQueryPath + "/Payment-getDistrict.json";
    _paymentMap["getWarehouse"]           = tpccQueryPath + "/Payment-getWarehouse.json";
    _paymentMap["insertHistory"]          = tpccQueryPath + "/Payment-insertHistory.json";
    _paymentMap["updateDistrictBalance"]  = tpccQueryPath + "/Payment-updateDistrictBalance.json";
    _paymentMap["updateBCCustomer"]       = tpccQueryPath + "/Payment-updateBCCustomer.json";
    _paymentMap["updateGCCustomer"]       = tpccQueryPath + "/Payment-updateGCCustomer.json";
    _paymentMap["updateWarehouseBalance"] = tpccQueryPath + "/Payment-updateWarehouseBalance.json";
  }

  void SetUp() {
    loadTables();

    i_customer_size  = loadTable(Customer)->size();
    i_orders_size    = loadTable(Orders)->size();
    i_orderLine_size = loadTable(OrderLine)->size();
    i_warehouse_size = loadTable(Warehouse)->size();
    i_newOrder_size  = loadTable(NewOrder)->size();
    i_district_size  = loadTable(District)->size();
    i_item_size      = loadTable(Item)->size();
    i_stock_size     = loadTable(Stock)->size();
    i_history_size   = loadTable(History)->size();
  }

  void TearDown() {
    io::ResourceManager::getInstance().clear();
  }

protected:
  typedef std::map<std::string, std::string> file_map_t;
  typedef struct item_info_t { int i_id; int i_w_id; int quantity; } ItemInfo;
  typedef std::vector<ItemInfo> item_info_list_t;

  void loadTables() {
    executeAndWait(loadFromFile("test/tpcc/load_tpcc_tables.json"));
  }

  void assureQueryFilesExist(const file_map_t& map) {
    for (auto& value : map) {
      std::ifstream s(value.second);
      if (!s)
        throw std::runtime_error("cannot read from file \"" + value.second + "\"");
      s.close();
    }
  }

  void doDelivery(int w_id, int d_id, int o_carrier_id, std::string date);
  void doNewOrder(int w_id, int d_id, int c_id, int o_carrier_id, std::string ol_dist_info, bool rollback,
                  std::string date, item_info_list_t items);
  void doOrderStatus(int w_id, int d_id, int c_id, std::string c_last);
  void doPayment(int w_id, int d_id, int c_id, std::string c_last, int c_w_id, int c_d_id, bool bc_customer, std::string date);
  Json::Value doStockLevel(int w_id, int d_id, int threshold);

  file_map_t _deliveryMap, _newOrderMap, _orderStatusMap, _paymentMap, _stockLevelMap;
  size_t i_customer_size, i_orders_size, i_orderLine_size, i_warehouse_size, i_newOrder_size,
         i_district_size, i_item_size, i_stock_size, i_history_size;

  const std::string _commit = "{\"operators\": {\"commit\": {\"type\": \"Commit\"}}}";
  const std::string _rollback = "{\"operators\": {\"rollback\": {\"type\": \"Rollback\"}}}";

  enum TpccTable { Customer, Orders, OrderLine, Warehouse, NewOrder, District, Item, Stock, History };

  static storage::c_atable_ptr_t loadTable(const TpccTable table, tx::transaction_id_t tid = hyrise::tx::UNKNOWN) {
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

    return executeAndWait("{\"operators\": {\"load\": {\"type\": \"TableLoad\", \"table\": \"" + tableName + "\"}" +
                          ", \"validate\": {\"type\": \"ValidatePositions\"}}, \"edges\": [[\"load\", \"validate\"]]}", 1, nullptr, tid);
  }
};

TEST_F(TpccStoredProceduresTest, LoadTables) {
  auto customer = loadTable(Customer);
  EXPECT_LT(0, customer->size()) << "invalid customer table";

  auto orders = loadTable(Orders);
  EXPECT_LT(0, orders->size()) << "invalid orders table";
  
  auto orderLine = loadTable(OrderLine);
  EXPECT_LT(0, orderLine->size()) << "invalid order_line table";
  
  auto warehouse = loadTable(Warehouse);
  EXPECT_LT(0, warehouse->size()) << "invalid warehouse table";
  
  auto newOrder = loadTable(NewOrder);
  EXPECT_LT(0, newOrder->size()) << "invalid new_order table";
  
  auto district = loadTable(District);
  EXPECT_LT(0, district->size()) << "invalid district table";
  
  auto item = loadTable(Item);
  EXPECT_LT(0, item->size()) << "invalid item table";
  
  auto stock = loadTable(Stock);
  EXPECT_LT(0, stock->size()) << "invalid stock table";
  
  auto history = loadTable(History);
  EXPECT_LT(0, history->size()) << "invalid history table";
}

//TEST_F(TpccStoredProceduresTest, DeliveryTest) {
void TpccStoredProceduresTest::doDelivery(int w_id, int d_id, int o_carrier_id, std::string date) {
  assureQueryFilesExist(_deliveryMap);

  parameter_map_t map;
  setParameter(map, "w_id", w_id);
  setParameter(map, "d_id", d_id);
  setParameter(map, "o_carrier_id", o_carrier_id);
  setParameter(map, "date", date);
  
  auto tid = getNewTXContext().tid;
  
  // getNewOrder
  auto t1 = executeAndWait(loadParameterized(_deliveryMap["getNewOrder"], map), 1, nullptr, tid);
  ASSERT_GE(t1->size(), 1);
  const int no_o_id = t1->getValue<int>("NO_O_ID", 0);
  setParameter(map, "no_o_id", no_o_id);

  // getCId
  auto t2 = executeAndWait(loadParameterized(_deliveryMap["getCId"], map), 1, nullptr, tid);
  ASSERT_EQ(t2->size(), 1);
  const int c_id = t2->getValue<int>("O_C_ID", 0);
  setParameter(map, "c_id", c_id);

  //sumOLAmount
  auto t3 = executeAndWait(loadParameterized(_deliveryMap["sumOLAmount"], map), 1, nullptr, tid);
  ASSERT_EQ(1, t3->size());
  ASSERT_EQ(1, t3->columnCount());
  const float ol_total = t3->getValue<float>(0, 0);
  setParameter(map, "ol_total", ol_total);

  executeAndWait(loadParameterized(_deliveryMap["deleteNewOrder"], map), 1, nullptr, tid); 
  executeAndWait(loadParameterized(_deliveryMap["updateOrders"], map), 1, nullptr, tid);
  executeAndWait(loadParameterized(_deliveryMap["updateOrderLine"], map), 1, nullptr, tid);
  executeAndWait(loadParameterized(_deliveryMap["updateCustomer"], map), 1, nullptr, tid);
  executeAndWait(_commit, 1, nullptr, tid);

  //simple size checks on tables:
  EXPECT_EQ(i_orders_size, loadTable(Orders)->size()) << "number of rows in ORDERS should not change";
  EXPECT_EQ(i_orderLine_size, loadTable(OrderLine)->size()) << "number of rows in ORDER-LINE should not change";
  EXPECT_EQ(i_customer_size, loadTable(Customer)->size()) << "number of rows in CUSTOMER should not change";
  
  EXPECT_EQ(i_newOrder_size, loadTable(NewOrder)->size() + 1) << "number of rows in New-Order should be exactly one less";
}

//TEST_F(TpccStoredProceduresTest, NewOrderTest) {
void TpccStoredProceduresTest::doNewOrder(int w_id, int d_id, int c_id, int o_carrier_id, std::string ol_dist_info, bool rollback,
                                 std::string date, item_info_list_t items) {
  const int all_local = std::all_of(items.cbegin(), items.cend(), [&](ItemInfo item) { return item.i_w_id == w_id; });
  //look for dublicates
  std::sort(items.begin(), items.end(), [](ItemInfo i1, ItemInfo i2) { return i1.i_id <= i2.i_id; });
  auto dublicates = std::unique(items.begin(), items.end(), [](ItemInfo i1, ItemInfo i2) { return i1.i_id == i2.i_id; });
  ASSERT_EQ(dublicates, items.end()) << "i_ids should not contain dublicates!";

  const int ol_cnt = items.size();

  assureQueryFilesExist(_newOrderMap);

  parameter_map_t map;
  setParameter(map, "w_id", w_id);
  setParameter(map, "d_id", d_id);
  setParameter(map, "c_id", c_id);
  setParameter(map, "date", date);
  setParameter(map, "o_carrier_id", o_carrier_id);
  setParameter(map, "o_ol_cnt", ol_cnt);
  setParameter(map, "all_local", all_local);
  setParameter(map, "ol_dist_info", ol_dist_info);

  auto tid = getNewTXContext().tid;

  typedef struct item_data_t { float price; std::string name; std::string data; } ItemData;
  std::vector<ItemData> itemData(ol_cnt);

// getNewOrder
  for (int i = 0; i < ol_cnt; ++i) {
    setParameter(map, "i_id", items.at(i).i_id);
    auto t1 = executeAndWait(loadParameterized(_newOrderMap["getItemInfo"], map), 1, nullptr, tid);
    ASSERT_EQ(t1->size(), 1);
    itemData[i].price = t1->getValue<float>("I_PRICE", 0);
    itemData[i].name = t1->getValue<std::string>("I_NAME", 0);
    itemData[i].data = t1->getValue<std::string>("I_DATA", 0);
  }

  auto t2 = executeAndWait(loadParameterized(_newOrderMap["getWarehouseTaxRate"], map), 1, nullptr, tid);
  ASSERT_EQ(t2->size(), 1);
  const float w_tax = t2->getValue<float>("W_TAX", 0);

  auto t3 = executeAndWait(loadParameterized(_newOrderMap["getDistrict"], map), 1, nullptr, tid);
  ASSERT_EQ(t3->size(), 1);
  const float d_tax = t3->getValue<float>("D_TAX", 0);
  const int o_id = t3->getValue<int>("D_NEXT_O_ID", 0);
  setParameter(map, "o_id", o_id);

  auto t4 = executeAndWait(loadParameterized(_newOrderMap["getCustomer"], map), 1, nullptr, tid);
  ASSERT_EQ(t4->size(), 1);
  const float c_discount = t4->getValue<float>("C_DISCOUNT", 0);
  const std::string c_last = t4->getValue<std::string>("C_LAST", 0);
  const std::string c_credit = t4->getValue<std::string>("C_CREDIT", 0);

  setParameter(map, "d_next_o_id", o_id + 1);
  executeAndWait(loadParameterized(_newOrderMap["incrementNextOrderId"], map), 1, nullptr, tid);
  executeAndWait(loadParameterized(_newOrderMap["createOrder"], map), 1, nullptr, tid);
  executeAndWait(loadParameterized(_newOrderMap["createNewOrder"], map), 1, nullptr, tid);

  int s_quantity;
  int s_ytd;
  int s_order_cnt, s_remote_cnt;
  std::string s_data, s_dist;
  int ol_supply_w_id, ol_i_id, ol_quantity;
  float ol_amount, total = 0;

  for (int i = 0; i < ol_cnt; ++i) {
    setParameter(map, "ol_number", i + 1);
    ol_supply_w_id = items.at(i).i_w_id;
    setParameter(map, "ol_supply_w_id", ol_supply_w_id);
    ol_i_id = items.at(i).i_id;
    setParameter(map, "ol_i_id", ol_i_id);
    ol_quantity = items.at(i).quantity;
    setParameter(map, "ol_quantity", ol_i_id);

    auto t5 = executeAndWait(loadParameterized(_newOrderMap["getStockInfo"], map), 1, nullptr, tid);
    ASSERT_EQ(t5->size(), 1);

    s_ytd = t5->getValue<int>("S_YTD", 0);
    s_ytd += ol_quantity;
    setParameter(map, "s_ytd", s_ytd);

    s_quantity = t5->getValue<int>("S_QUANTITY", 0);
    s_order_cnt = t5->getValue<int>("S_ORDER_CNT", 0);
    if (s_quantity >= ol_quantity + 10) {
      s_quantity = s_quantity - ol_quantity;
    }
    else {
      s_quantity = s_quantity + 91 - ol_quantity;
      ++s_order_cnt;
    }
    setParameter(map, "s_order_cnt", s_order_cnt);
    setParameter(map, "s_quantity", s_quantity);

    s_remote_cnt = t5->getValue<int>("S_REMOTE_CNT", 0);
    setParameter(map, "s_remote_cnt", s_remote_cnt);

    std::string s_data = t5->getValue<std::string>("S_DATA", 0);
    std::string s_dist = t5->getValue<std::string>(5, 0);

    ol_amount = ol_quantity * itemData.at(i).price;
    setParameter(map, "ol_amount", ol_amount);

    total += ol_amount;
    
    ASSERT_EQ(t5->columnCount(), 6);

    executeAndWait(loadParameterized(_newOrderMap["updateStock"], map), 1, nullptr, tid);
    executeAndWait(loadParameterized(_newOrderMap["createOrderLine"], map), 1, nullptr, tid);
  }

  if (rollback) {
    executeAndWait(_rollback, 1, nullptr, tid);
  
    EXPECT_EQ(i_stock_size, loadTable(Stock)->size()) << "number of rows in STOCK should not change";
    EXPECT_EQ(i_newOrder_size, loadTable(NewOrder)->size()) << "number of rows in NEW-ORDER should not change";
    EXPECT_EQ(i_orders_size, loadTable(Orders)->size()) << "number of rows in ORDERS should not change";
    EXPECT_EQ(i_orderLine_size, loadTable(OrderLine)->size()) << "number of rows in ORDER-LINES should not change";
    
    return;
  }

  total *=  (1 - c_discount) * (1 + w_tax + d_tax);

  executeAndWait(_commit, 1, nullptr, tid);

  //simple size checks on tables:
  EXPECT_EQ(i_stock_size, loadTable(Stock)->size()) << "number of rows in STOCK should not change";
  
  EXPECT_EQ(i_newOrder_size, loadTable(NewOrder)->size() - 1) << "number of rows in NEW-ORDER should be exactly one more";
  EXPECT_EQ(i_orders_size, loadTable(Orders)->size() - 1) << "number of rows in ORDERS should be exactly one more";
  EXPECT_EQ(i_orderLine_size, loadTable(OrderLine)->size() - ol_cnt) << "number of rows in ORDER-LINES should be exactly " << ol_cnt << " more";
}

//TEST_F(TpccStoredProceduresTest, OrderStatusTest) {
void TpccStoredProceduresTest::doOrderStatus(int w_id, int d_id, int c_id, std::string c_last) {
  const bool selectByLastName = (c_last.size() > 0);
  
  assureQueryFilesExist(_orderStatusMap);

  parameter_map_t map;
  setParameter(map, "w_id", w_id);
  setParameter(map, "d_id", d_id);
  setParameter(map, "c_id", c_id);
  setParameter(map, "c_last", c_last);

  auto tid = getNewTXContext().tid;

  if (!selectByLastName) {
    auto t1 = executeAndWait(loadParameterized(_orderStatusMap["getCustomerByCId"], map), 1, nullptr, tid);
    ASSERT_EQ(t1->size(), 1);
    ASSERT_EQ(c_id,  t1->getValue<int>("C_ID", 0));
    c_id = t1->getValue<int>("C_ID", 0);
  }
  else {
    auto t1 = executeAndWait(loadParameterized(_orderStatusMap["getCustomersByLastName"], map), 1, nullptr, tid);
    ASSERT_GE(t1->size(), 1);
    const size_t chosenOne = (t1->size() - 1) / 2;
    c_id = t1->getValue<int>("C_ID", chosenOne);
  }
  setParameter(map, "c_id", c_id);

  auto t2 = executeAndWait(loadParameterized(_orderStatusMap["getLastOrder"], map), 1, nullptr, tid);
  ASSERT_GE(t2->size(), 1);
  const int o_id = t2->getValue<int>("O_ID", 0);
  setParameter(map, "o_id", o_id);

  auto t3 = executeAndWait(loadParameterized(_orderStatusMap["getOrderLines"], map), 1, nullptr, tid);
  ASSERT_GE(t3->size(), 1);

  executeAndWait(_commit, 1, nullptr, tid);
}

//TEST_F(TpccStoredProceduresTest, PaymentTest) {
void TpccStoredProceduresTest::doPayment(int w_id, int d_id, int c_id, std::string c_last, int c_w_id, int c_d_id, bool bc_customer, std::string date) {
  assureQueryFilesExist(_paymentMap);

  const bool selectByLastName = (c_last.size() > 0);
  const float h_amount = 314.16;

  parameter_map_t map;
  setParameter(map, "w_id", w_id);
  setParameter(map, "d_id", d_id);
  setParameter(map, "h_amount", h_amount);
  setParameter(map, "c_w_id", c_w_id);
  setParameter(map, "c_d_id", c_d_id);
  setParameter(map, "c_id", c_id);
  setParameter(map, "c_last", c_last);
  setParameter(map, "date", date);

  auto tid = getNewTXContext().tid;
 
  float c_balance, c_ytd_payment;
  int c_payment_cnt;
  std::string c_data;

  if (!selectByLastName) {
    auto t1 = executeAndWait(loadParameterized(_paymentMap["getCustomerByCId"], map), 1, nullptr, tid);
    ASSERT_EQ(t1->size(), 1);
    setParameter(map, "c_id", t1->getValue<int>("C_ID", 0));
    c_balance = t1->getValue<float>("C_BALANCE", 0);
    c_payment_cnt = t1->getValue<int>("C_PAYMENT_CNT", 0);
    c_ytd_payment = t1->getValue<float>("C_YTD_PAYMENT", 0);
    c_data = t1->getValue<std::string>("C_DATA", 0);
  }
  else {
    auto t1 = executeAndWait(loadParameterized(_paymentMap["getCustomersByLastName"], map), 1, nullptr, tid);
    ASSERT_GE(t1->size(), 1);
    const size_t chosenOne = (t1->size() - 1) / 2;
    setParameter(map, "c_id", t1->getValue<int>("C_ID", chosenOne));
    c_balance = t1->getValue<float>("C_BALANCE", 0);
    c_payment_cnt = t1->getValue<int>("C_PAYMENT_CNT", 0);
    c_ytd_payment = t1->getValue<float>("C_YTD_PAYMENT", 0);
    c_data = t1->getValue<std::string>("C_DATA", 0);
  }
  
  setParameter(map, "c_balance", c_balance - h_amount);
  setParameter(map, "c_payment_cnt", c_payment_cnt + 1);
  setParameter(map, "c_ytd_payment", c_ytd_payment + h_amount);
  setParameter(map, "c_data", c_data);

  auto t2 = executeAndWait(loadParameterized(_paymentMap["getWarehouse"], map), 1, nullptr, tid);
  ASSERT_EQ(t2->size(), 1);
  const float w_ytd = t2->getValue<float>("W_YTD", 0);
  const std::string w_name = t2->getValue<std::string>("W_NAME", 0);
  setParameter(map, "w_ytd", w_ytd + h_amount);
  
  auto t3 = executeAndWait(loadParameterized(_paymentMap["getDistrict"], map));
  ASSERT_EQ(t3->size(), 1);
  const float d_ytd = t3->getValue<float>("D_YTD", 0);
  const std::string d_name = t3->getValue<std::string>("D_NAME", 0);
  setParameter(map, "d_ytd", d_ytd + h_amount);

  setParameter(map, "h_data", w_name + "    " + d_name);

  executeAndWait(loadParameterized(_paymentMap["updateWarehouseBalance"], map), 1, nullptr, tid);
  executeAndWait(loadParameterized(_paymentMap["updateDistrictBalance"], map), 1, nullptr, tid);
  
  if (bc_customer)
    executeAndWait(loadParameterized(_paymentMap["updateBCCustomer"], map), 1, nullptr, tid);
  else
    executeAndWait(loadParameterized(_paymentMap["updateGCCustomer"], map), 1, nullptr, tid);
  
  executeAndWait(loadParameterized(_paymentMap["insertHistory"], map), 1, nullptr, tid);

  executeAndWait(_commit, 1, nullptr, tid);
  
  //simple size checks on tables:
  EXPECT_EQ(i_customer_size, loadTable(Customer)->size()) << "number of rows in CUSTOMER should not change";
  EXPECT_EQ(i_warehouse_size, loadTable(Warehouse)->size()) << "number of rows in WAREHOUSE should not change";
  EXPECT_EQ(i_district_size, loadTable(District)->size()) << "number of rows in DISTRICT should not change";
  
  EXPECT_EQ(i_history_size, loadTable(History)->size() - 1) << "number of rows in NEW-ORDER should be exactly one more";
}

Json::Value TpccStoredProceduresTest::doStockLevel(int w_id, int d_id, int threshold) {
  Json::Value root;
  root["W_ID"] = w_id;
  root["D_ID"] = d_id;
  root["threshold"] = threshold;

  Json::StyledWriter writer;
    
  const auto s = executeStoredProcedureAndWait("TPCC-StockLevel", writer.write(root));
  Json::Reader reader;
  Json::Value response;
  if (!reader.parse(s, response))
    throw std::runtime_error("Failed to parse json");

  return response;
}




//===========================Delivery Tests

TEST_F(TpccStoredProceduresTest, Delivery_W1D1) {
  //        (w_id, d_id, o_carrier_id, date      );
  doDelivery(1   , 1   , 1           , niceDate  );
}

//===========================New Order Tests

TEST_F(TpccStoredProceduresTest, NewOrder_W1D1C1) {
  //        (w_id, d_id, c_id, o_carrier_id, ol_dist_info, rollback, date     , {{i_id, i_w_id, quantity}});
  doNewOrder(1   , 1   , 1   , 1           , "info"      , false   , niceDate , {{1   , 1     , 10      },
                                                                                 {5   , 1     , 30      },
                                                                                 {7   , 1     , 90      }});
}

TEST_F(TpccStoredProceduresTest, NewOrder_W1D1C1rollback) {
  //        (w_id, d_id, c_id, o_carrier_id, ol_dist_info, rollback, date     , {{i_id, i_w_id, quantity}});
  doNewOrder(1   , 1   , 1   , 1           , "info"      , true   , niceDate ,  {{3   , 1     , 40      },
                                                                                 {11  , 1     , 20      },
                                                                                 {5   , 1     , 10      }});
}

//===========================Order Status Tests

TEST_F(TpccStoredProceduresTest, OrderStatus_W1D1C1) {
  //           (w_id, d_id, c_id, c_last         );
  doOrderStatus(1   , 1   , 1   , ""             );
}

TEST_F(TpccStoredProceduresTest, OrderStatus_W1D1BARBARBAR) {
  //           (w_id, d_id, c_id, c_last         );
  doOrderStatus(1   , 1   , 1   , "BARBARBAR"    );
}

//============================Payment Tests

TEST_F(TpccStoredProceduresTest, Payment_W1D1C1localBC) {
  //       (w_id, d_id, c_id, c_last        , c_w_id, c_d_id, bc_customer, date      );
  doPayment(1   , 1   , 1   , ""            , 1     , 1     , true       , niceDate  );
}

TEST_F(TpccStoredProceduresTest, Payment_W1D1BARBARATIONlocalGC) {
  //       (w_id, d_id, c_id, c_last        , c_w_id, c_d_id, bc_customer, date      );
  doPayment(1   , 1   , 1   , "BARBARATION" , 1     , 1     , false      , niceDate  );
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
  //                                 (w_id, d_id, threshold);
  const auto& response = doStockLevel(1   , 1   , 90       );

  ASSERT_EQ(1, getValuei(response, "W_ID"));
  ASSERT_EQ(1, getValuei(response, "D_ID"));
  ASSERT_EQ(90, getValuei(response, "threshold"));
  ASSERT_EQ(7, getValuei(response, "low_stock"));
}

TEST_F(TpccStoredProceduresTest, StockLevel_W1D1T50) {
  //                                 (w_id, d_id, threshold);
  const auto& response = doStockLevel(1   , 1   , 50       );

  ASSERT_EQ(1, getValuei(response, "W_ID"));
  ASSERT_EQ(1, getValuei(response, "D_ID"));
  ASSERT_EQ(50, getValuei(response, "threshold"));
  ASSERT_EQ(1, getValuei(response, "low_stock"));
}

} } // namespace hyrise::access

