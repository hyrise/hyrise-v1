// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TpccStoredProcedure.h"

#include <storage/AbstractTable.h>
#include <helper/HttpHelper.h>

#include <io/TransactionManager.h>
#include <access/storage/TableLoad.h>
#include <access/SimpleTableScan.h>
#include <access/ProjectionScan.h>

#include <access/tx/ValidatePositions.h>
#include <access/tx/Commit.h>

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
    return Json::Value();
  }
  auto request = urldecode(body_data["data"]);
  std::cout << request << std::endl;

  Json::Value request_data;
  Json::Reader reader;

  if (!reader.parse(request, request_data)) {
    throw std::runtime_error("Failed to parse json");
  }

  return request_data;
}

storage::c_atable_ptr_t TpccStoredProcedure::loadTpccTable(std::string name, const tx::TXContext& tx) {
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

void TpccStoredProcedure::commit(tx::TXContext tx) {
  Commit commit;
  commit.setTXContext(tx);
  commit.execute();
}

}} // namespace hyrise::access

