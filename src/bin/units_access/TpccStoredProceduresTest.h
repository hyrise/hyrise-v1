// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "io/shortcuts.h"
#include "testing/test.h"

#include "helper.h"
#include <fstream>
#include <json.h>

namespace hyrise {
namespace access {

namespace {
typedef struct item_t {
  int id;
  int wid;
  int quantity;
} ItemInfo;
typedef std::vector<ItemInfo> item_list_t;
}  // namespace

class TpccError : public std::runtime_error {
 public:
  TpccError(const std::string& what) : std::runtime_error(what) {}
};

class TpccStoredProceduresTest : public AccessTest {
 public:
  void SetUp();
  void TearDown();

 protected:
  void loadTables();

  size_t i_customer_size, i_orders_size, i_orderLine_size, i_warehouse_size, i_newOrder_size, i_district_size,
      i_item_size, i_stock_size, i_history_size;


  enum TpccTable {
    Customer,
    Orders,
    OrderLine,
    Warehouse,
    NewOrder,
    District,
    Item,
    Stock,
    History
  };

  static storage::c_atable_ptr_t getTable(const TpccTable& table);

  static Json::Value doStoredProcedure(const Json::Value& data, const std::string& procedureName);

  static void assureFieldExists(const Json::Value& data, const std::string& name);
  static int getValuei(const Json::Value& data, const std::string& name);
  static float getValuef(const Json::Value& data, const std::string& name);
  static std::string getValues(const Json::Value& data, const std::string& name);

  template <class T>
  std::string toString(const T& in) {
    std::ostringstream os;
    os << in;
    return os.str();
  }
};
}
}  // namespace hyrise::access
