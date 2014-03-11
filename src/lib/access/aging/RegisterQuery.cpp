// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "RegisterQuery.h"

#include <io/StorageManager.h>
#include <access/system/QueryParser.h>
#include <access/aging/QueryManager.h>
#include <access/aging/expressions/SelectExpression.h>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<RegisterQuery>("RegisterQuery");
}

RegisterQuery::RegisterQuery(const std::string& name) :
  PlanOperation(),
  _name(name) {}

void RegisterQuery::executePlanOperation() {
  auto& qm = QueryManager::instance();

  qm.registerQuery(_name, _select);
  _select = nullptr;

  output = input; // don't stand in the way of calculation, pass everything on
}

void RegisterQuery::selectExpression(const std::shared_ptr<aging::SelectExpression>& select) {
  _select = select;
}

std::shared_ptr<PlanOperation> RegisterQuery::parse(const Json::Value& data) {
  if (!data.isMember("name"))
    throw std::runtime_error("A name is needed to register a query for");
  const auto& name = data["name"].asString();

  auto rq = std::make_shared<RegisterQuery>(name);

  if (!data.isMember("select"))
    throw std::runtime_error("An empty select expression makes no sense");
  rq->_select = aging::SelectExpression::parse(data["select"]);

  return rq;
}

} } // namespace hyrise::access

