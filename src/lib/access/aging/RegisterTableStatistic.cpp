// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "RegisterTableStatistic.h"

#include <iostream>

#include <io/StorageManager.h>

#include <access/aging/TableStatistic.h>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<RegisterTableStatistic>("RegisterTableStatistic");
} // namespace


RegisterTableStatistic::RegisterTableStatistic(const std::string& table, const std::string& field) :
  PlanOperation(),
  _table(table),
  _field(field) {}

void RegisterTableStatistic::executePlanOperation() {
  auto& sm = *io::StorageManager::getInstance();

  const auto& table = sm.getTable(_table);
  const auto& field = table->numberOfColumn(_field);

  const auto& statisticTable = input.nthOf<storage::AbstractTable>(0);

  if (sm.hasStatistic(_table, _field))
    return;

  const auto statistic = std::make_shared<TableStatistic>(table, field, statisticTable);
  sm.setStatisticFor(_table, _field, statistic);
}

std::shared_ptr<PlanOperation> RegisterTableStatistic::parse(const Json::Value &data) {
  if (!data.isMember("table"))
    throw std::runtime_error("A table name must be specified for the RegisterTableStatistic");
  const auto& table = data["table"].asString();

  if (!data.isMember("field"))
    throw std::runtime_error("A field name must be specified for the RegisterTableStatistic");
  const auto& field = data["field"].asString();

  return std::make_shared<RegisterTableStatistic>(table, field);
}

} } // namespace hyrise::access

