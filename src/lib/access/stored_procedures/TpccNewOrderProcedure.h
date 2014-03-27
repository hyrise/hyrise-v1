// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "TpccStoredProcedure.h"

namespace hyrise {
namespace access {

class TpccNewOrderProcedure : public TpccStoredProcedure {
 public:
  TpccNewOrderProcedure(net::AbstractConnection* connection);

  virtual Json::Value execute();
  virtual void setData(const Json::Value& data);

  static std::string name();
  virtual const std::string vname();

 private:
  typedef struct item_info_t {
    int id;
    int w_id;
    int quantity;
    int s_quantity;
    int s_order_cnt;
    int s_remote_cnt;
    float price;
    std::string name;
    bool original;
    float amount() const { return quantity * price; }
  } ItemInfo;
  typedef std::vector<ItemInfo> item_info_list_t;

  // queries
  void createNewOrder();
  void createOrderLine(const ItemInfo& item, int ol_number);
  void createOrder();
  storage::c_atable_ptr_t getCustomer();
  storage::c_atable_ptr_t getDistrict();
  storage::c_atable_ptr_t getItemInfo(int i_id);
  storage::c_atable_ptr_t getStockInfo(const ItemInfo& item);
  storage::c_atable_ptr_t getWarehouseTaxRate();
  void incrementNextOrderId(const storage::atable_ptr_t& districtRow);
  void updateStock(const storage::atable_ptr_t& stockRow, const ItemInfo& item);

  int allLocal() const;

  int _w_id;
  int _d_id;
  int _c_id;
  int _o_id;
  size_t _ol_cnt;
  int _carrier_id;
  int _ol_supply_w_id;
  std::string _ol_dist_info;
  std::string _date;
  int _all_local;
  item_info_list_t _items;
};
}
}  // namespace hyrise::access
