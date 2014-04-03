// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccStoredProceduresTest.h"

#include "io/shortcuts.h"
#include "testing/test.h"

#include "helper.h"
#include <fstream>
#include <json.h>

#include <io/ResourceManager.h>

namespace hyrise {
namespace access {

void TpccStoredProceduresTest::SetUp() {
  loadTables();

  i_customer_size = getTable(Customer)->size();
  i_orders_size = getTable(Orders)->size();
  i_orderLine_size = getTable(OrderLine)->size();
  i_warehouse_size = getTable(Warehouse)->size();
  i_newOrder_size = getTable(NewOrder)->size();
  i_district_size = getTable(District)->size();
  i_item_size = getTable(Item)->size();
  i_stock_size = getTable(Stock)->size();
  i_history_size = getTable(History)->size();
}

void TpccStoredProceduresTest::TearDown() { io::ResourceManager::getInstance().clear(); }

void TpccStoredProceduresTest::loadTables() { executeAndWait(loadFromFile("test/tpcc/load_tpcc_tables.json")); }

storage::c_atable_ptr_t TpccStoredProceduresTest::getTable(const TpccTable& table) {
  std::string tableName;
  std::string fileName;
  switch (table) {
    case Customer:
      tableName = "CUSTOMER";
      break;
    case Orders:
      tableName = "ORDERS";
      break;
    case OrderLine:
      tableName = "ORDER_LINE";
      break;
    case Warehouse:
      tableName = "WAREHOUSE";
      break;
    case NewOrder:
      tableName = "NEW_ORDER";
      break;
    case District:
      tableName = "DISTRICT";
      break;
    case Item:
      tableName = "ITEM";
      break;
    case Stock:
      tableName = "STOCK";
      break;
    case History:
      tableName = "HISTORY";
      break;
  }

  return executeAndWait("{\"operators\": {\"load\": {\"type\": \"GetTable\", \"name\": \"" + tableName + "\"}" +
                        ", \"validate\": {\"type\": \"ValidatePositions\"}}, \"edges\": [[\"load\", \"validate\"]]}");
}

Json::Value TpccStoredProceduresTest::doStoredProcedure(const Json::Value& data, const std::string& procedureName) {
  Json::StyledWriter writer;
  const auto s = executeStoredProcedureAndWait(procedureName, writer.write(data));

  Json::Reader reader;
  Json::Value response;

  if (!reader.parse(s, response))
    throw TpccError(s);
  return response;
}

void TpccStoredProceduresTest::assureFieldExists(const Json::Value& data, const std::string& name) {
  if (!data.isMember(name))
    throw std::runtime_error("\'" + name + "\' should be set but is not");
}

int TpccStoredProceduresTest::getValuei(const Json::Value& data, const std::string& name) {
  assureFieldExists(data, name);
  return data[name].asInt();
}

float TpccStoredProceduresTest::getValuef(const Json::Value& data, const std::string& name) {
  assureFieldExists(data, name);
  return data[name].asFloat();
}

std::string TpccStoredProceduresTest::getValues(const Json::Value& data, const std::string& name) {
  assureFieldExists(data, name);
  return data[name].asString();
}
}
}  // namespace hyrise::access
