// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccNewOrderProcedure.h"

#include <iomanip>

#include <storage/AbstractTable.h>
#include <access/expressions/pred_EqualsExpression.h>
#include <access/expressions/pred_CompoundExpression.h>
#include <access/JoinScan.h>
#include <access/GroupByScan.h>

namespace hyrise { namespace access {

namespace {
  auto _ = net::Router::registerRoute<TpccNewOrderProcedure>("/TPCC-NewOrder/");
} // namespace


TpccNewOrderProcedure::TpccNewOrderProcedure(net::AbstractConnection* connection) :
  TpccStoredProcedure(connection) {
}

void TpccNewOrderProcedure::setData(Json::Value& data) {
  _w_id =         assureMemberExists(data, "W_ID").asInt();
  _d_id =         assureMemberExists(data, "D_ID").asInt();
  _c_id =         assureMemberExists(data, "C_ID").asInt();
  _carrier_id =   assureMemberExists(data, "O_CARRIER_ID").asInt();
  _ol_dist_info = assureMemberExists(data, "OL_DIST_INFO").asString();

  const auto itemInfo = assureMemberExists(data, "items");
  _ol_cnt = itemInfo.size();
  for (int i = 0; i < _ol_cnt; ++i) {
    ItemInfo info;
    info.id =       assureMemberExists(itemInfo[i],"I_ID").asInt();
    info.w_id =     assureMemberExists(itemInfo[i],"I_W_ID").asInt();
    info.quantity = assureMemberExists(itemInfo[i],"quantity").asInt();
    _items.push_back(info);
  }
}

std::string TpccNewOrderProcedure::name() {
  return "TPCC-NewOrder";
}

const std::string TpccNewOrderProcedure::vname() {
  return "TPCC-NewOrder";
}

int TpccNewOrderProcedure::allLocal() const {
  if (std::all_of(_items.cbegin(), _items.cend(), [&] (const ItemInfo& item) { return item.w_id == _w_id; }))
    return 1;
  return 0;
}

Json::Value TpccNewOrderProcedure::execute() {
  _date = getDate();
  _all_local = allLocal();

  _rollback = false;
  for (auto& item : _items) {
    auto tItem = getItemInfo(item.id);
    if (tItem->size() == 0) {
      ;//TODO not found condition
    }
    item.price = tItem->getValue<float>("I_PRICE", 0);
    item.name = tItem->getValue<std::string>("I_NAME", 0);
    item.data = tItem->getValue<std::string>("I_DATA", 0);
  }

  auto tWarehouse = getWarehouseTaxRate();
  //TODO no warehouse
  const float w_tax = tWarehouse->getValue<float>("W_TAX", 0);

  auto tDistrict = getDistrict();
  //TODO no district
  const float d_tax = tDistrict->getValue<float>("D_TAX", 0);
  _o_id = tDistrict->getValue<int>("D_NEXT_O_ID", 0);

  auto tCustomer = getCustomer();
  //TODO no customer
  const float c_discount = tCustomer->getValue<float>("C_DISCOUNT", 0);
  const std::string c_last = tCustomer->getValue<std::string>("C_LAST", 0);
  const std::string c_credit = tCustomer->getValue<std::string>("C_CREDIT", 0);

  incrementNextOrderId();
  createOrder();
  createNewOrder();

  int s_ytd;
  std::string s_data, s_dist;
  float total = 0;

  for (int i = 0; i < _ol_cnt; ++i) {
    auto& item = _items.at(i);
    const int quantity = _items.at(i).quantity;

    auto t5 = getStockInfo(item.w_id, item.id);
    //TODO no stock info

    s_ytd = t5->getValue<int>("S_YTD", 0);
    s_ytd += quantity; //TODO WHAT???

    int s_quantity = t5->getValue<int>("S_QUANTITY", 0);
    item.s_order_cnt = t5->getValue<int>("S_ORDER_CNT", 0);

    if (s_quantity >= quantity + 10) {
      s_quantity = s_quantity - quantity;
    }
    else {
      s_quantity = s_quantity + 91 - quantity;
      ++item.s_order_cnt;
    }
    item.s_quantity = s_quantity;
    item.s_remote_cnt = t5->getValue<int>("S_REMOTE_CNT", 0);

    std::string s_data = t5->getValue<std::string>("S_DATA", 0);

    std::ostringstream os;
    os << "S_DIST_" << std::setw(2) << std::setfill('0') << std::right << _d_id;
    const std::string s_dist_name = os.str(); //e.g. S_DIST_01
    std::cout << "<<<<<<" << s_dist_name << std::endl;
    std::string s_dist = t5->getValue<std::string>(s_dist_name, 0);

    total += quantity * _items.at(i).price;

    updateStock(item);
    createOrderLine(item, i + 1);
  }

  if (_rollback)
    rollback(); //this can skip more actions (2.4.2.3)!
  else
    commit();


  Json::Value result;
  result["W_ID"] = _w_id;
  result["D_ID"] = _d_id;
  result["C_ID"] = _c_id;
  result["C_LAST"] = c_last;
  result["C_CREDIT"] = c_credit;
  result["C_DISCOUNT"] = c_discount;
  result["W_TAX"] = w_tax;
  result["D_TAX"] = d_tax;
  result["O_OL_CNT"] = _ol_cnt;
  result["O_ID"] = _o_id;
  result["O_ENTRY_D"] = _date;

  if (_rollback) {
    result["total-amount"] = "Item number is not valid";
    return result;
  }
  //needed for what?
  total *=  (1 - c_discount) * (1 + w_tax + d_tax);
  result["total-amount"] = total;

  for (int i = 0; i < _ol_cnt; ++i) {
    Json::Value item;
    const auto& cur = _items.at(i);
    item["OL_SUPPLY_W_ID"] = cur.w_id;
    item["OL_I_ID"] = cur.id;
    item["I_NAME"] = cur.name;
    item["OL_QUANTITY"] = cur.quantity;
    item["S_QUANTITY"] = cur.s_quantity;
    if (cur.bc)
      item["brand-generic"] = "BC";
    else
      item["brand-generic"] = "GC";
    item["I_PRICE"] = cur.quantity;
    item["OL_AMOUNT"] = cur.price * cur.quantity;
  }

  return result;
}

void TpccNewOrderProcedure::createNewOrder() {
  auto newOrder = std::const_pointer_cast<AbstractTable>(getTpccTable("NEW_ORDER"));

  auto newRow = newOrder->copy_structure(nullptr, true, 1);
  newRow->setValue<hyrise_int_t>(0, 0, _o_id);
  newRow->setValue<hyrise_int_t>(1, 0, _d_id);
  newRow->setValue<hyrise_int_t>(2, 0, _w_id);
  
  insert(newOrder, newRow);
}

void TpccNewOrderProcedure::createOrderLine(const ItemInfo& item, const int ol_number) {
  auto orderLine = std::const_pointer_cast<AbstractTable>(getTpccTable("ORDER_LINE"));

  auto newRow = orderLine->copy_structure(nullptr, true, 1);
  newRow->setValue<hyrise_int_t>(0, 0, _o_id);
  newRow->setValue<hyrise_int_t>(1, 0, _d_id);
  newRow->setValue<hyrise_int_t>(2, 0, _w_id);
  newRow->setValue<hyrise_int_t>(3, 0, ol_number);
  newRow->setValue<hyrise_int_t>(4, 0, item.id);
  newRow->setValue<hyrise_int_t>(5, 0, item.w_id);
  newRow->setValue<hyrise_string_t>(6, 0, _date);
  newRow->setValue<hyrise_int_t>(7, 0, item.quantity);
  newRow->setValue<hyrise_float_t>(8, 0, item.amount());
  newRow->setValue<hyrise_string_t>(9, 0, _ol_dist_info);
  
  insert(orderLine, newRow);
}

void TpccNewOrderProcedure::createOrder() {
  auto orders = std::const_pointer_cast<AbstractTable>(getTpccTable("ORDERS"));

  auto newRow = orders->copy_structure(nullptr, true, 1);
  newRow->setValue<hyrise_int_t>(0, 0, _o_id);
  newRow->setValue<hyrise_int_t>(1, 0, _d_id);
  newRow->setValue<hyrise_int_t>(2, 0, _w_id);
  newRow->setValue<hyrise_int_t>(3, 0, _c_id);
  newRow->setValue<hyrise_string_t>(4, 0, _date);
  newRow->setValue<hyrise_int_t>(5, 0, _carrier_id);
  newRow->setValue<hyrise_int_t>(6, 0, _ol_cnt);
  newRow->setValue<hyrise_int_t>(7, 0, _all_local);
  
  insert(orders, newRow);
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getCustomer() {
  auto customer = getTpccTable("CUSTOMER");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(customer, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(customer, "C_ID", _c_id));
  auto validated = selectAndValidate(customer, connectAnd(expressions));

  return validated;
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getDistrict() {
  auto district = getTpccTable("DISTRICT");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "D_ID", _d_id));
  auto validated = selectAndValidate(district, connectAnd(expressions));

  return validated;
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getItemInfo(int i_id) {
  auto item = getTpccTable("ITEM");

  auto expr = new EqualsExpression<hyrise_int_t>(item, "I_ID", i_id);
  auto validated = selectAndValidate(item, expr);

  return validated;
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getStockInfo(int w_id, int i_id) {
  auto district = getTpccTable("DISTRICT");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "S_W_ID", w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "S_I_ID", i_id));
  auto validated = selectAndValidate(district, connectAnd(expressions));

  return validated;
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getWarehouseTaxRate() {
  auto district = getTpccTable("WAREHOUSE");

  auto expr = new EqualsExpression<hyrise_int_t>(district, "W_ID", _w_id);
  auto validated = selectAndValidate(district, expr);

  return validated;
}

void TpccNewOrderProcedure::incrementNextOrderId() {
  auto district = getTpccTable("DISTRICT");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "D_ID", _d_id));
  auto validated = selectAndValidate(district, connectAnd(expressions));

  Json::Value updates;
  updates["D_NEXT_O_ID"] = _o_id + 1;
  update(validated, updates);
}

void TpccNewOrderProcedure::updateStock(const ItemInfo& item) {
  auto district = getTpccTable("STOCK");

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "S_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<hyrise_int_t>(district, "S_I_ID", item.id));
  auto validated = selectAndValidate(district, connectAnd(expressions));

  Json::Value updates;
  updates["S_QUANTITY"] = item.quantity;
  updates["S_ORDER_CNT"] = item.s_order_cnt;
  updates["S_REMOTE_CNT"] = item.s_remote_cnt;
  updates["S_QUANTITY"] = item.quantity;
  update(validated, updates);
}

} } // namespace hyrise::access

