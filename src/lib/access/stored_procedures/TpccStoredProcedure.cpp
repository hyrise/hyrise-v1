// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccStoredProcedure.h"

#include <ctime>

#include <storage/AbstractTable.h>
#include <helper/HttpHelper.h>

#include <io/ResourceManager.h>
#include <io/TransactionManager.h>
#include <access.h>

namespace hyrise { namespace access {

TpccStoredProcedure::TpccStoredProcedure(net::AbstractConnection* connection) :
  _connection(connection) {
}

void TpccStoredProcedure::operator()() {
  try {
    auto d = data();
    setData(d);
  
    auto result = execute();
    Json::StyledWriter writer;
    _connection->respond(writer.write(result));
  }
  catch (std::runtime_error e) {
    _connection->respond(e.what());
  }
}

Json::Value TpccStoredProcedure::data() {
  if (!_connection->hasBody())
    throw std::runtime_error("message has no body");

  std::map<std::string, std::string> body_data = parseHTTPFormData(_connection->getBody());
  auto data = body_data.find("data");
  if (data == body_data.end()) {
    throw std::runtime_error("No data object in json");
  }
  auto request = urldecode(body_data["data"]);

  Json::Value request_data;
  Json::Reader reader;

  if (!reader.parse(request, request_data)) {
    throw std::runtime_error("Failed to parse json");
  }

  return request_data;
}

namespace {
  typedef std::map<std::string, std::string> file_map_t;

  static const std::string tpccTableDir = "test/tpcc/";
  static const std::string tpccDelimiter = ",";
  const file_map_t tpccHeaderLocation = {{"DISTRICT"  , tpccTableDir + "district_header.tbl"},
                                         {"WAREHOUSE" , tpccTableDir + "warehouse_header.tbl"},
                                         {"CUSTOMER"  , tpccTableDir + "customer_header.tbl"},
                                         {"HISTORY"   , tpccTableDir + "history_header.tbl"},
                                         {"ORDERS"    , tpccTableDir + "orders_header.tbl"},
                                         {"NEW_ORDER" , tpccTableDir + "new_order_header.tbl"},
                                         {"STOCK"     , tpccTableDir + "stock_header.tbl"},
                                         {"ORDER_LINE", tpccTableDir + "order_line_header.tbl"}};
  const file_map_t tpccDataLocation = {{"DISTRICT"  , tpccTableDir + "district.csv"},
                                       {"WAREHOUSE" , tpccTableDir + "warehouse.csv"},
                                       {"CUSTOMER"  , tpccTableDir + "customer.csv"},
                                       {"HISTORY"   , tpccTableDir + "history.csv"},
                                       {"ORDERS"    , tpccTableDir + "orders.csv"},
                                       {"NEW_ORDER" , tpccTableDir + "new_order.csv"},
                                       {"STOCK"     , tpccTableDir + "stock.csv"},
                                       {"ORDER_LINE", tpccTableDir + "order_line.csv"}};
} // namespace

Json::Value TpccStoredProcedure::assureMemberExists(Json::Value data, const std::string name) {
  if (!data.isMember(name))
    throw std::runtime_error("Parameter \"" + name + "\" is not defined");
  return data[name];
}

storage::c_atable_ptr_t TpccStoredProcedure::getTpccTable(std::string name, const tx::TXContext& tx) {
  TableLoad load;
  load.setTXContext(tx);
  load.setTableName(name);
  load.setHeaderFileName(tpccHeaderLocation.at(name));
  load.setFileName(tpccDataLocation.at(name));
  load.setDelimiter(tpccDelimiter);
  load.execute();

  return load.getResultTable();
}
 
storage::c_atable_ptr_t TpccStoredProcedure::selectAndValidate(storage::c_atable_ptr_t table, SimpleExpression *expr, const tx::TXContext& tx) {
  SimpleTableScan select;
  select.addInput(table);
  select.setTXContext(tx);
  select.setPredicate(expr);
  select.execute();
  
  ValidatePositions validate;
  validate.setTXContext(tx);
  validate.addInput(select.getResultTable());
  validate.execute();

  return validate.getResultTable();
}

storage::c_atable_ptr_t TpccStoredProcedure::project(storage::c_atable_ptr_t table, field_name_set_t fields, const tx::TXContext& tx) {
  ProjectionScan project;
  project.setTXContext(tx);
  project.addInput(table);
  for (auto& field : fields)
    project.addField(field);
  project.execute();
  return project.getResultTable();
}

tx::TXContext TpccStoredProcedure::startTransaction() {
  return tx::TransactionManager::beginTransaction();
}

void TpccStoredProcedure::insert(storage::atable_ptr_t table, storage::atable_ptr_t rows, const tx::TXContext& tx) {
  InsertScan insert;
  insert.setTXContext(tx);
  insert.addInput(table);
  insert.setInputData(rows);
  insert.execute();
}

void TpccStoredProcedure::deleteRows(storage::c_atable_ptr_t rows, const tx::TXContext& tx) {
  DeleteOp del;
  del.setTXContext(tx);
  del.addInput(rows);
  del.execute();
}

void TpccStoredProcedure::update(storage::c_atable_ptr_t rows, Json::Value data, const tx::TXContext& tx) {
  PosUpdateScan update;
  update.setTXContext(tx);
  update.addInput(rows);

  update.setRawData(data);
  update.execute();
}

storage::c_atable_ptr_t TpccStoredProcedure::sort(storage::c_atable_ptr_t table, field_name_t field, bool ascending, const tx::TXContext& tx) {
  SortScan sort;
  sort.addInput(table);
  sort.setSortField(field);
  sort.execute();
  return sort.getResultTable();
}

SimpleExpression* TpccStoredProcedure::connectAnd(expr_list_t expressions) {
  if (expressions.size() == 0)
    throw std::runtime_error("cannot connect no (0) expressions");
  else if (expressions.size() == 1)
    return expressions.at(0);

  auto lastAnd = new CompoundExpression(expressions.at(0), expressions.at(1), AND);
  for (size_t i = 2; i < expressions.size(); ++i)
    lastAnd = new CompoundExpression(expressions.at(i), lastAnd, AND);
  return lastAnd;
}

void TpccStoredProcedure::commit(tx::TXContext tx) {
  Commit commit;
  commit.setTXContext(tx);
  commit.execute();
}

void TpccStoredProcedure::rollback(tx::TXContext tx) {
  Rollback rb;
  rb.setTXContext(tx);
  rb.execute();
}

//source http://stackoverflow.com/questions/5438482
// Get current date/time, format is YYYY-MM-DD
std::string TpccStoredProcedure::getDate() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[40];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return buf;
}

} } // namespace hyrise::access

