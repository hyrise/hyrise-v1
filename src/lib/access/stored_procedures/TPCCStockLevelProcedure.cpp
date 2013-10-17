// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TPCCStockLevelProcedure.h"

#include <stdexcept>

#include <storage/AbstractTable.h>
#include <io/TransactionManager.h>
#include <access/storage/TableLoad.h>
#include <access/SimpleTableScan.h>
#include <access/ProjectionScan.h>
#include <access/expressions/pred_EqualsExpression.h>
#include <access/expressions/pred_CompoundExpression.h>

#include <access/tx/ValidatePositions.h>
#include <access/tx/Commit.h>

namespace hyrise { namespace access {

TPCCStockLevelProcedure::TPCCStockLevelProcedure(int w_id, int d_id, int threshold) :
  _w_id(w_id),
  _d_id(d_id),
  _threshold(threshold) {
    // maybe check correctness of parameters
}

TPCCStockLevelProcedure::TPCCStockLevelProcedure() :
  _w_id(0),
  _d_id(0),
  _threshold(0) {
  throw std::runtime_error("cannot call this constructor for TPCCStockLevelProcedure");
}

TPCCStockLevelProcedure::TPCCStockLevelProcedure(const TPCCStockLevelProcedure& procedure) :
  _w_id(0),
  _d_id(0),
  _threshold(0) {
  throw std::runtime_error("cannot call this constructor for TPCCStockLevelProcedure");
}


result_map_t TPCCStockLevelProcedure::execute() {
  // Transaction
  _tx = startTransaction();

  auto t1 = getOId();
  _next_o_id = t1->getValue<int>("D_NEXT_O_ID", 0);
  t1->print();

  auto t2 = getStockCount();
  t2->print();

  commit(_tx);

  // Output

  int low_stock = 0;
  if (t2->size() != 0) 
    low_stock = t2->getValue<int>(0, 0);

  result_map_t result_map;
  result_map["W_ID"] = toString(_w_id);
  result_map["D_ID"] = toString(_d_id);
  result_map["threshold"] = toString(_threshold);
  result_map["low_stock"] = toString(low_stock);

  return result_map;
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::getOId() {
/*{
 * "operators": {
 *   "load": {
 *     "type": "GetTable",
 *     "name": "DISTRICT"
 *   },
 *   "select": {
 *     "type": "SimpleTableScan",
 *     "predicates": [
 *        {"type": "AND"},
 *        {"type": "EQ", "in": 0, "f": "D_W_ID", "vtype": 0, "value": %(w_id)d},
 *        {"type": "EQ", "in": 0, "f": "D_ID", "vtype": 0 , "value": %(d_id)d}
 *      ]
 *    },
 *    "project": {
 *      "type": "ProjectionScan",
 *      "fields": ["D_NEXT_O_ID"]
 *    }
 *  },
 *  "edges": [["load", "select"], ["select", "project"]]
 *}*/
  auto district = loadAndValidate("DISTRICT", _tx);

  SimpleTableScan select;
  select.setTXContext(_tx);
  select.addInput(district);
  auto expr_w_id = new EqualsExpression<hyrise_int_t>(district, "D_W_ID", _w_id);
  auto expr_d_id = new EqualsExpression<hyrise_int_t>(district, "D_ID", _d_id);
  auto expr_and = new CompoundExpression(expr_w_id, expr_d_id, AND);
  select.setPredicate(expr_and);
  select.execute();

  auto result = project(select.getResultTable(), {"D_NEXT_O_ID"}, _tx);

  return result;
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::getStockCount() {
  auto order_line = loadAndValidate("ORDER_LINE", _tx);
  auto stock = loadAndValidate("STOCK", _tx);

  //TODO
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::loadAndValidate(std::string name, const tx::TXContext& tx) {
  TableLoad load;
  load.setTXContext(tx);
  load.setTableName(name);
  load.setHeaderFileName(tpccHeaderLocation.at(name));
  load.setFileName(tpccDataLocation.at(name));
  load.setDelimiter(tpccDelimiter);
  load.execute();
  
  ValidatePositions validate;
  validate.setTXContext(tx);
  validate.addInput(load.getResultTable());
  validate.execute();

  return validate.getResultTable();
}

storage::c_atable_ptr_t TPCCStockLevelProcedure::project(storage::c_atable_ptr_t table, field_name_set_t fields, const tx::TXContext& tx) {
  ProjectionScan project;
  project.setTXContext(tx);
  project.addInput(table);
  for (auto& field : fields)
    project.addField(field);
  project.execute();
  return project.getResultTable();
}

tx::TXContext TPCCStockLevelProcedure::startTransaction() {
  return tx::TransactionManager::beginTransaction();
}

void TPCCStockLevelProcedure::commit(tx::TXContext tx) {
  Commit commit;
  commit.setTXContext(tx);
  commit.execute();
}

}} // namespace hyrise::access

