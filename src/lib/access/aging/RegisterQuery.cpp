// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "RegisterQuery.h"

#include <iostream>

#include <io/StorageManager.h>
#include <access/system/QueryParser.h>
#include <access/aging/QueryManager.h>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<RegisterQuery>("RegisterQuery");
}

void RegisterQuery::executePlanOperation() {
  auto& qm = QueryManager::instance();
  auto& sm = *io::StorageManager::getInstance();

  param_list_t paramVector;
  for (const auto& table : _fields) {
    const auto& tableName = table.first;
    sm.assureExists(tableName);
    const auto& t = sm.get<storage::AbstractTable>(tableName); // test if its a table

    const auto& fields = table.second;
    for (const auto& field: fields) {
      field_t fieldId = t->numberOfColumn(field);
      paramVector.push_back({t->id(), fieldId});
      std::cout << tableName << "." << field << "[" << fieldId << "], ";
    }
  }
  std::cout << std::endl;
  qm.registerQuery(_name, paramVector);

  output = input; // don't stand in the way of calculation, pass everything on
}

std::shared_ptr<PlanOperation> RegisterQuery::parse(const Json::Value &data) {
  std::shared_ptr<RegisterQuery> rq = std::make_shared<RegisterQuery>();

  if (!data.isMember("name"))
    throw std::runtime_error("A name is needed to register a query for");
  rq->_name = data["name"].asString();

  if (data.isMember("parameters")) {
    const auto& params = data["parameters"];
    for (unsigned i = 0; i < params.size(); ++i) {
      const auto& param = params[i];
      if (!param.isMember("table"))
        throw std::runtime_error("You need to specify a table in which the fields should be");
      const std::string& table = param["table"].asString();

      if (param.isMember("fields")) {
        const auto& fields = param["fields"];
        field_list_t fv;
        for (unsigned j = 0; j < fields.size(); ++j)
          fv.push_back(fields[j].asString());
        rq->_fields.insert(std::make_pair(table, fv));
      }
    }
  }

  return rq;
}

} } // namespace hyrise::access

