// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AgingRun.h"

#include <iostream>

#include <storage/storage_types.h>
#include <storage/AgingStore.h>
#include <storage/AbstractStatistic.h>
#include <io/StorageManager.h>

#include <access/SimpleTableScan.h>
#include <access/expressions/pred_CompoundExpression.h>
#include <access/aging/QueryManager.h>
#include <access/aging/StatisticSnapshot.h>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<AgingRun>("AgingRun");
} // namespace

AgingRun::AgingRun(const std::string& table) :
  PlanOperation(),
  _tableName(table) {}

void AgingRun::executePlanOperation() {
  auto& sm = *io::StorageManager::getInstance();
  auto& qm = QueryManager::instance();

  storage::aging_store_ptr_t agingStore;
  const auto table = sm.getTable(_tableName);

  if (std::dynamic_pointer_cast<storage::AgingStore>(table) == nullptr) {
    std::cout << "table currently is no AgingStore - trying to fix this" << std::endl;
    const auto& store = checked_pointer_cast<storage::Store>(table);
    agingStore = std::make_shared<storage::AgingStore>(store);
    //TODO just replace like that?
  }
  else {
    agingStore = checked_pointer_cast<storage::AgingStore>(table);
  }

  const auto relevantQueries = qm.queriesOfTable(table);

  std::cout << "looking for available statistics" << std::endl;
  std::vector<storage::astat_ptr_t> statistics;
  for (storage::field_t col = 0; col < table->columnCount(); ++col) {
    if (sm.hasStatistic(_tableName, table->nameOfColumn(col))) {
      statistics.push_back(sm.getStatisticFor(_tableName, table->nameOfColumn(col)));
      //statistics.back()->table(agingStore);
      std::cout << "\tfound for column: " << table->nameOfColumn(col) << std::endl;
    }
  }

  std::cout << "create statistic snapshots" << std::endl;
  std::vector<std::shared_ptr<StatisticSnapshot>> snapshots;
  for (size_t i = 0; i < statistics.size(); ++i)
    snapshots.push_back(std::make_shared<StatisticSnapshot>(*statistics.at(i)));

  std::cout << "create classification queries" << std::endl;
  std::vector<SimpleExpression*> expressions;
  for (const auto& query : relevantQueries) {
    std::map<storage::field_t, std::vector<storage::value_id_t>> vids;
    for (const auto& snapshot : snapshots) {
      auto& vidList = vids[snapshot->field()];
      snapshot->valuesDo(query, [&vidList](storage::value_id_t vid, bool hot) { if (hot) vidList.push_back(vid); });
    }
    const auto expr = qm.selectExpressionOf(query)->expression(agingStore, vids);
    if (expr != nullptr)
      expressions.push_back(expr);
  }

  SimpleExpression* expr = nullptr;
  for (const auto& expression : expressions) {
    if (expr == nullptr)
      expr = expression;
    else
      expr = new CompoundExpression(expr, expression, OR);
  }

  if (expr == nullptr) {
    std::cout << "no aging information - aborting" << std::endl;
    return;
  }

  std::cout << "execute the classification queries" << std::endl;
  SimpleTableScan scan;
  scan.addInput(agingStore);
  scan.setPredicate(expr);
  scan.setProducesPositions(true);
  scan.execute();

  std::cout << "do the aging!" << std::endl; //TODO columnwise
  const auto& pointerCalculator = checked_pointer_cast<const storage::PointerCalculator>(scan.getResultTable());
  pointerCalculator->print();
  pos_list_t posList(pointerCalculator->getPositions()->begin(), pointerCalculator->getPositions()->end());

  if (posList.size() == 0) {
    std::cout << "no hot tuples - aborting" << std::endl;
    return;
  }
  //TODO is already sorted? std::sort(posList.begin(), posList.end());
  agingStore->age(posList);

  if (table != agingStore) {
    std::cout << "replace table with aging store" << std::endl;
    sm.replace(_tableName, agingStore);
  }

  std::cout << "register AgingIdices" << std::endl;
  for (const auto& snapshot : snapshots) {
    snapshot->table(agingStore);
    const auto agingIndex = std::make_shared<storage::AgingIndex>(snapshot);
    sm.setAgingIndexFor(_tableName, agingStore->nameOfColumn(snapshot->field()), agingIndex);
  }
}

std::shared_ptr<PlanOperation> AgingRun::parse(const Json::Value &data) {
  if (!data.isMember("table"))
    throw std::runtime_error("A table must be specified for the AgingRun");
  const auto table = data["table"].asString();

  return std::make_shared<AgingRun>(table);
}

} } // namespace hyrise::access

