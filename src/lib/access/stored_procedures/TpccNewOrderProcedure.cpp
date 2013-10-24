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
  _w_id = data["W_ID"].asInt();
  _d_id = data["D_ID"].asInt();
  _c_id = data["C_ID"].asInt();
  _carrier_id = data["O_CARRIER_ID"].asInt();
  _ol_dist_info = data["OL_DIST_ID"].asString();
  _rollback = data["rollback"].asBool();

  const auto iteminfo = data["items"];
  _ol_cnt = data.size();
  for (int i = 0; i < _ol_cnt; ++i) {
    ItemInfo info;
    info.id = iteminfo[i]["I_ID"].asInt();
    info.w_id = iteminfo[i]["I_W_ID"].asInt();
    info.quantity = iteminfo[i]["quantity"].asInt();
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

  _tx = startTransaction();

  for (auto& item : _items) {
    auto t1 = getItemInfo(item.id);
    if (t1->size() == 0) {
      ;//TODO not found condition
    }
    item.price = t1->getValue<float>("I_PRICE", 0);
    item.name = t1->getValue<std::string>("I_NAME", 0);
    item.data = t1->getValue<std::string>("I_DATA", 0);
  }

  auto t2 = getWarehouseTaxRate();
  const float w_tax = t2->getValue<float>("W_TAX", 0);

  auto t3 = getDistrict();
  const float d_tax = t3->getValue<float>("D_TAX", 0);
  _o_id = t3->getValue<int>("D_NEXT_O_ID", 0);

  auto t4 = getCustomer();
  const float c_discount = t4->getValue<float>("C_DISCOUNT", 0);
  const std::string c_last = t4->getValue<std::string>("C_LAST", 0);
  const std::string c_credit = t4->getValue<std::string>("C_CREDIT", 0);

  incrementNextOrderId();
  createOrder();
  createNewOrder();

  int s_ytd;
  //int s_order_cnt, s_remote_cnt; what for?
  std::string s_data, s_dist;
  float total = 0;

  for (int i = 0; i < _ol_cnt; ++i) {
    auto& item = _items.at(i);
    const int quantity = _items.at(i).quantity;

    auto t5 = getStockInfo(item.w_id, item.id);

    s_ytd = t5->getValue<int>("S_YTD", 0);
    s_ytd += quantity; //WHAT??? and maybe use it ;)

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
    item.s_remote_cnt = t5->getValue<int>("S_REMOTE_CNT", 0); //needed for?

    std::string s_data = t5->getValue<std::string>("S_DATA", 0);
    std::string s_dist = t5->getValue<std::string>(5, 0); //really index? :/

    total += quantity * _items.at(i).price;

    updateStock(item);
    createOrderLine(item, i + 1);
  }

  if (_rollback)
    rollback(_tx); //this can skip more actions (2.4.2.3)!
  else
    commit(_tx);



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
  auto newOrder = std::const_pointer_cast<AbstractTable>(getTpccTable("NEW_ORDER", _tx));

  auto newRow = newOrder->copy_structure(nullptr, false, 1);
  newRow->setValue<int>(0, 0, _o_id);
  newRow->setValue<int>(1, 0, _d_id);
  newRow->setValue<int>(2, 0, _w_id);
  
  insert(newOrder, newRow, _tx);
}

void TpccNewOrderProcedure::createOrderLine(const ItemInfo& item, const int ol_number) {
  auto orderLine = std::const_pointer_cast<AbstractTable>(getTpccTable("ORDER_LINE", _tx));

  auto newRow = orderLine->copy_structure(nullptr, false, 1);
  newRow->setValue<int>(0, 0, _o_id);
  newRow->setValue<int>(1, 0, _d_id);
  newRow->setValue<int>(2, 0, _w_id);
  newRow->setValue<int>(3, 0, ol_number);
  newRow->setValue<int>(4, 0, item.id);
  newRow->setValue<int>(5, 0, item.w_id);
  newRow->setValue<std::string>(6, 0, _date);
  newRow->setValue<int>(7, 0, item.quantity);
  newRow->setValue<float>(8, 0, item.amount());
  newRow->setValue<std::string>(9, 0, _ol_dist_info);
  
  insert(orderLine, newRow, _tx);
}

void TpccNewOrderProcedure::createOrder() {
  auto orders = std::const_pointer_cast<AbstractTable>(getTpccTable("ORDERS", _tx));

  auto newRow = orders->copy_structure(nullptr, false, 1);
  newRow->setValue<int>(0, 0, _o_id);
  newRow->setValue<int>(1, 0, _d_id);
  newRow->setValue<int>(2, 0, _w_id);
  newRow->setValue<int>(3, 0, _c_id);
  newRow->setValue<std::string>(4, 0, _date);
  newRow->setValue<int>(5, 0, _carrier_id);
  newRow->setValue<int>(6, 0, _ol_cnt);
  newRow->setValue<int>(7, 0, _all_local);
  
  insert(orders, newRow, _tx);
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getCustomer() {
  auto customer = getTpccTable("CUSTOMER", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(customer, "C_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_D_ID", _d_id));
  expressions.push_back(new EqualsExpression<int>(customer, "C_ID", _c_id));
  auto validated = selectAndValidate(customer, connectAnd(expressions), _tx);

  auto result = project(validated, {"C_DISCOUNT", "C_LAST", "C_CREDIT"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getDistrict() {
  auto district = getTpccTable("DISTRICT", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(district, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(district, "D_ID", _d_id));
  auto validated = selectAndValidate(district, connectAnd(expressions), _tx);

  auto result = project(validated, {"D_TAX", "D_NEXT_O_ID"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getItemInfo(int i_id) {
  auto item = getTpccTable("ITEM", _tx);

  auto expr = new EqualsExpression<int>(item, "I_ID", i_id);
  auto validated = selectAndValidate(item, expr, _tx);

  auto result = project(validated, {"I_PRICE", "I_NAME", "I_DATA"}, _tx);
  return result;
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getStockInfo(int w_id, int i_id) {
  auto district = getTpccTable("DISTRICT", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(district, "S_W_ID", w_id));
  expressions.push_back(new EqualsExpression<int>(district, "S_I_ID", i_id));
  auto validated = selectAndValidate(district, connectAnd(expressions), _tx);

  std::ostringstream os;
  os << std::setw(2) << std::setfill('0') << std::right << _d_id;
  const std::string s_dist_name = os.str();
  auto result = project(validated, {"S_QUANTITY", "S_DATA", "S_YTD", "S_ORDER_CNT", "S_REMOTE_CNT", s_dist_name}, _tx);

  return result;
}

storage::c_atable_ptr_t TpccNewOrderProcedure::getWarehouseTaxRate() {
  auto district = getTpccTable("WAREHOUSE", _tx);

  auto expr = new EqualsExpression<int>(district, "W_ID", _w_id);
  auto validated = selectAndValidate(district, expr, _tx);

  auto result = project(validated, {"W_TAX"}, _tx);

  return result;
}

void TpccNewOrderProcedure::incrementNextOrderId() {
  auto district = getTpccTable("DISTRICT", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(district, "D_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(district, "D_ID", _d_id));
  auto validated = selectAndValidate(district, connectAnd(expressions), _tx);

  Json::Value updates;
  updates["D_NEXT_O_ID"] = _o_id + 1;
  update(validated, updates, _tx);
}

void TpccNewOrderProcedure::updateStock(const ItemInfo& item) {
  auto district = getTpccTable("STOCK", _tx);

  expr_list_t expressions;
  expressions.push_back(new EqualsExpression<int>(district, "S_W_ID", _w_id));
  expressions.push_back(new EqualsExpression<int>(district, "S_I_ID", item.id));
  auto validated = selectAndValidate(district, connectAnd(expressions), _tx);

  Json::Value updates;
  updates["S_QUANTITY"] = item.quantity;
  updates["S_ORDER_CNT"] = item.s_order_cnt;
  updates["S_REMOTE_CNT"] = item.s_remote_cnt;
  updates["S_QUANTITY"] = item.quantity;
  update(validated, updates, _tx);
}

} } // namespace hyrise::access

