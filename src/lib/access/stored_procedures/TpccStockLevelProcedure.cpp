// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccStockLevelProcedure.h"

#include <storage/AbstractTable.h>
#include <access.h>
#include "access/CompoundIndexScan.h"
#include "access/CompoundIndexRangeScan.h"
#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

namespace {
auto _ = net::Router::registerRoute<TpccStockLevelProcedure>("/TPCC-StockLevel/");
}  // namespace


TpccStockLevelProcedure::TpccStockLevelProcedure(net::AbstractConnection* connection)
    : TpccStoredProcedure(connection) {}

void TpccStockLevelProcedure::setData(const Json::Value& data) {
  _w_id = assureMemberExists(data, "W_ID").asInt();
  _d_id = assureIntValueBetween(data, "D_ID", 1, 10);
  _threshold = assureIntValueBetween(data, "threshold", 10, 20);
}

std::string TpccStockLevelProcedure::name() { return "TPCC-StockLevel"; }

const std::string TpccStockLevelProcedure::vname() { return "TPCC-StockLevel"; }

Json::Value TpccStockLevelProcedure::execute() {
  auto t1 = getOId();
  if (t1->size() == 0) {
    std::ostringstream os;
    os << "no district " << _d_id << " for warehouse " << _w_id;
    throw std::runtime_error(os.str());
  }
  _next_o_id = t1->getValue<hyrise_int_t>("D_NEXT_O_ID", 0);

  auto t2 = getStockCount();
  // commit();

  // Output
  int low_stock = t2;
  // if (t2->size() != 0)
  //   low_stock = t2->getValue<hyrise_int_t>(0, 0);

  Json::Value result;
  result["W_ID"] = _w_id;
  result["D_ID"] = _d_id;
  result["threshold"] = _threshold;
  result["low_stock"] = low_stock;
  return result;
}

storage::c_atable_ptr_t TpccStockLevelProcedure::getOId() {
  CompoundIndexScan scan;
  scan.addInput(getTpccTable("DISTRICT"));
  scan.setMainIndex("mcidx__DISTRICT__main__D_W_ID__D_ID");
  scan.setDeltaIndex("mcidx__DISTRICT__delta__D_W_ID__D_ID");
  scan.addPredicate<hyrise_int_t>("D_W_ID", _w_id);
  scan.addPredicate<hyrise_int_t>("D_ID", _d_id);
  scan.setValidation(true);
  scan.setUniqueIndex(true);
  scan.setTXContext(_tx);
  scan.execute();

  return scan.getResultTable();
}

hyrise_int_t TpccStockLevelProcedure::getStockCount() {

  // order_line: load, select and validate

  /* "STOCK_LEVEL": {
        "getOId": "SELECT D_NEXT_O_ID FROM DISTRICT WHERE D_W_ID = ? AND D_ID = ?",
        "getStockCount": """
            SELECT COUNT(DISTINCT(OL_I_ID)) FROM ORDER_LINE, STOCK
            WHERE OL_W_ID = ?
              AND OL_D_ID = ?
              AND OL_O_ID < ?
              AND OL_O_ID >= ?
              AND S_W_ID = ?
              AND S_I_ID = OL_I_ID
              AND S_QUANTITY < ?
        """,
    },
    */

  auto order_line = getTpccTable("ORDER_LINE");
  const auto min_o_id = _next_o_id - 20;
  const auto max_o_id = _next_o_id;  // D_NEXT_O_ID shoult be greater than every O_ID

  // perform a range scan on the compound PK index of Order Line
  CompoundIndexRangeScan ol_idx_scan;
  ol_idx_scan.addInput(order_line);
  ol_idx_scan.setMainIndex("mcidx__ORDER_LINE__main__OL_W_ID__OL_D_ID__OL_O_ID");
  ol_idx_scan.setDeltaIndex("mcidx__ORDER_LINE__delta__OL_W_ID__OL_D_ID__OL_O_ID");
  ol_idx_scan.addPredicate<hyrise_int_t>("OL_W_ID", _w_id);
  ol_idx_scan.addPredicate<hyrise_int_t>("OL_D_ID", _w_id);
  ol_idx_scan.addRangeMinPredicate<hyrise_int_t>("OL_O_ID", min_o_id);
  ol_idx_scan.addRangeMaxPredicate<hyrise_int_t>("OL_O_ID", max_o_id);
  ol_idx_scan.setValidation(true);
  ol_idx_scan.setUniqueIndex(false);
  ol_idx_scan.setTXContext(_tx);
  ol_idx_scan.execute();
  auto validated_ol = checked_pointer_cast<const storage::PointerCalculator>(ol_idx_scan.getResultTable());
  if (validated_ol->size() == 0) {
    throw std::runtime_error("Error in stock level, did not find any order lines");
  }

  pos_list_t* results = new pos_list_t;
  auto stock = getTpccTable("STOCK");

  results->reserve(32);

  std::set<hyrise_int_t> distinct_item_ids;

  auto s_id_col = validated_ol->numberOfColumn("OL_I_ID");
  auto quant_col = stock->numberOfColumn("S_QUANTITY");

  // check the _threshold for all matching orderlines (20 * ~10), 200 Items.. by checking the index on stock
  for (size_t i = 0; i < validated_ol->size(); ++i) {
    auto s_id = validated_ol->getValue<hyrise_int_t>(s_id_col, i);
    CompoundIndexScan cis;
    cis.addInput(stock);
    cis.setMainIndex("mcidx__STOCK__main__S_W_ID__S_I_ID");
    cis.setDeltaIndex("mcidx__STOCK__delta__S_W_ID__S_I_ID");
    cis.addPredicate<hyrise_int_t>("S_W_ID", _w_id);
    cis.addPredicate<hyrise_int_t>("S_I_ID", s_id);
    cis.setValidation(true);
    cis.setUniqueIndex(true);
    cis.setTXContext(_tx);
    cis.execute();
    auto quant_table = cis.getResultTable();
    if (quant_table->size() != 1)
      throw std::runtime_error("StockCount for UniqueOL ID failed.");
    auto qt = quant_table->getValue<hyrise_int_t>(quant_col, 0);
    if (qt < _threshold) {
      results->push_back(validated_ol->getPositions()->at(i));
      distinct_item_ids.insert(s_id);
    }
  }
  return distinct_item_ids.size();
}
}
}  // namespace hyrise::access
