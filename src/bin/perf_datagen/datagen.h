// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <string>
#include <utility>
#include <map>
#include <vector>



static std::vector<std::pair<std::string, std::string> > warehouse_layout;
static std::vector<std::pair<std::string, std::string> > district_layout;
static std::vector<std::pair<std::string, std::string> > customer_layout;
static std::vector<std::pair<std::string, std::string> > item_layout;
static std::vector<std::pair<std::string, std::string> > order_layout;
static std::vector<std::pair<std::string, std::string> > order_line_layout;
static std::vector<std::pair<std::string, std::string> > stock_layout;
static std::vector<std::pair<std::string, std::string> > history_layout;
static std::vector<std::pair<std::string, std::string> > new_order_layout;

static std::map<std::string, std::vector<std::pair<std::string, std::string> > > layouts;

void loadHyriseLayouts() {
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_ID", "INTEGER"));
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_NAME", "STRING"));
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_STREET_1", "STRING"));
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_STREET_2", "STRING"));
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_CITY", "STRING"));
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_STATE", "STRING"));
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_ZIP", "STRING"));
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_TAX", "FLOAT"));
  warehouse_layout.push_back(std::pair<std::string, std::string>("W_YTD", "FLOAT"));

  layouts["warehouse"] = warehouse_layout;

  district_layout.push_back(std::pair<std::string, std::string>("D_ID", "INTEGER"));
  district_layout.push_back(std::pair<std::string, std::string>("D_W_ID", "INTEGER"));
  district_layout.push_back(std::pair<std::string, std::string>("D_NAME", "STRING"));
  district_layout.push_back(std::pair<std::string, std::string>("D_STREET_1", "STRING"));
  district_layout.push_back(std::pair<std::string, std::string>("D_STREET_2", "STRING"));
  district_layout.push_back(std::pair<std::string, std::string>("D_CITY", "STRING"));
  district_layout.push_back(std::pair<std::string, std::string>("D_STATE", "STRING"));
  district_layout.push_back(std::pair<std::string, std::string>("D_ZIP", "STRING"));
  district_layout.push_back(std::pair<std::string, std::string>("D_TAX", "FLOAT"));
  district_layout.push_back(std::pair<std::string, std::string>("D_YTD", "FLOAT"));
  district_layout.push_back(std::pair<std::string, std::string>("D_NEXT_O_ID", "INTEGER"));

  layouts["district"] = district_layout;

  customer_layout.push_back(std::pair<std::string, std::string>("C_ID", "INTEGER"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_D_ID", "INTEGER"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_W_ID", "INTEGER"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_FIRST", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_MIDDLE", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_LAST", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_STREET_1", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_STREET_2", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_CITY", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_STATE", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_ZIP", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_PHONE", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_SINCE", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_CREDIT", "STRING"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_CREDIT_LIM", "FLOAT"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_DISCOUNT", "FLOAT"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_BALANCE", "FLOAT"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_YTD_PAYMENT", "FLOAT"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_PAYMENT_CNT", "INTEGER"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_DELIVERY_CNT", "INTEGER"));
  customer_layout.push_back(std::pair<std::string, std::string>("C_DATA", "STRING"));

  layouts["customer"] = customer_layout;

  history_layout.push_back(std::pair<std::string, std::string>("H_C_ID", "INTEGER"));
  history_layout.push_back(std::pair<std::string, std::string>("H_C_D_ID", "INTEGER"));
  history_layout.push_back(std::pair<std::string, std::string>("H_C_W_ID", "INTEGER"));
  history_layout.push_back(std::pair<std::string, std::string>("H_D_ID", "INTEGER"));
  history_layout.push_back(std::pair<std::string, std::string>("H_W_ID", "INTEGER"));
  history_layout.push_back(std::pair<std::string, std::string>("H_DATE", "STRING"));
  history_layout.push_back(std::pair<std::string, std::string>("H_AMOUNT", "FLOAT"));
  history_layout.push_back(std::pair<std::string, std::string>("H_DATA", "STRING"));

  layouts["history"] = history_layout;

  new_order_layout.push_back(std::pair<std::string, std::string>("NO_O_ID", "INTEGER"));
  new_order_layout.push_back(std::pair<std::string, std::string>("NO_D_ID", "INTEGER"));
  new_order_layout.push_back(std::pair<std::string, std::string>("NO_W_ID", "INTEGER"));

  layouts["new_order"] = new_order_layout;

  order_layout.push_back(std::pair<std::string, std::string>("O_ID", "INTEGER"));
  order_layout.push_back(std::pair<std::string, std::string>("O_D_ID", "INTEGER"));
  order_layout.push_back(std::pair<std::string, std::string>("O_W_ID", "INTEGER"));
  order_layout.push_back(std::pair<std::string, std::string>("O_C_ID", "INTEGER"));
  order_layout.push_back(std::pair<std::string, std::string>("O_ENTRY_D", "STRING"));
  order_layout.push_back(std::pair<std::string, std::string>("O_CARRIER_ID", "INTEGER"));
  order_layout.push_back(std::pair<std::string, std::string>("O_OL_CNT", "INTEGER"));
  order_layout.push_back(std::pair<std::string, std::string>("O_ALL_LOCAL", "INTEGER"));

  layouts["order"] = order_layout;

  order_line_layout.push_back(std::pair<std::string, std::string>("OL_O_ID", "INTEGER"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_D_ID", "INTEGER"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_W_ID", "INTEGER"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_NUMBER", "INTEGER"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_I_ID", "INTEGER"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_SUPPLY_W_ID", "INTEGER"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_DELIVERY_D", "STRING"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_QUANTITY", "FLOAT"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_AMOUNT", "FLOAT"));
  order_line_layout.push_back(std::pair<std::string, std::string>("OL_DIST_INFO", "STRING"));

  layouts["order_line"] = order_line_layout;

  item_layout.push_back(std::pair<std::string, std::string>("I_ID", "INTEGER"));
  item_layout.push_back(std::pair<std::string, std::string>("I_IM_ID", "INTEGER"));
  item_layout.push_back(std::pair<std::string, std::string>("I_NAME", "STRING"));
  item_layout.push_back(std::pair<std::string, std::string>("I_PRICE", "FLOAT"));
  item_layout.push_back(std::pair<std::string, std::string>("I_DATA", "STRING"));

  layouts["item"] = item_layout;

  stock_layout.push_back(std::pair<std::string, std::string>("S_I_ID", "INTEGER"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_W_ID", "INTEGER"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_QUANTITY", "INTEGER"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_01", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_02", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_03", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_04", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_05", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_06", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_07", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_08", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_09", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DIST_10", "STRING"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_YTD", "INTEGER"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_ORDER_CNT", "INTEGER"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_REMOTE_CNT", "INTEGER"));
  stock_layout.push_back(std::pair<std::string, std::string>("S_DATA", "STRING"));

  layouts["stock"] = stock_layout;
}
