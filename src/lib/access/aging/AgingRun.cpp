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

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<AgingRun>("AgingRun");
} // namespace

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
    sm.replace(_tableName, agingStore);
  }
  else {
    agingStore = checked_pointer_cast<storage::AgingStore>(table);
  }

  const auto relevantQueries = qm.queriesOfTable(agingStore);

  std::cout << "looking for available statistics" << std::endl;
  std::vector<storage::astat_ptr_t> statistics;
  for (storage::field_t col = 0; col < table->columnCount(); ++col) {
    if (sm.hasStatistic(_tableName, table->nameOfColumn(col))) {
      statistics.push_back(sm.getStatisticFor(_tableName, table->nameOfColumn(col)));
      std::cout << "\tfound for column: " << table->nameOfColumn(col) << std::endl;
    }
  }

  std::cout << "create classification queries" << std::endl;
  std::vector<SimpleExpression*> expressions;
  for (const auto& query : relevantQueries) {
    std::cout << "QUERY: " << query << " (" << qm.getName(query) << ")" << std::endl;
    std::cout << "\tcollecting hot values" << std::endl;
    std::map<storage::field_t, std::vector<storage::value_id_t>> vids;
    for (const auto& statistic : statistics) {
      auto& vidList = vids[statistic->field()];
      std::cout << "\t(" << table->nameOfColumn(statistic->field()) << ")" << std::endl;
      statistic->valuesDo(query, [&vidList](storage::value_id_t vid, bool hot)
                                  { if (hot) vidList.push_back(vid); });
    }
    expressions.push_back(qm.selectExpressionOf(query)->expression(table, vids));
  }

  SimpleExpression* expr = nullptr;
  for (const auto& expression : expressions) {
    if (expression == nullptr)
      continue;

    /*SimpleTableScan scan;
    scan.addInput(table);
    scan.setPredicate(expression);
    scan.setProducesPositions(true);
    scan.execute();
    scan.getResultTable()->print();*/

    if (expr == nullptr)
      expr = expression;
    else {
      expr = new CompoundExpression(expr, expression, OR);
    }
  }

  if (expr == nullptr) //nothing to do no information
    return;

  SimpleTableScan scan;
  scan.addInput(table);
  scan.setPredicate(expr);
  scan.setProducesPositions(true);
  scan.execute();

  const auto& pointerCalculator = checked_pointer_cast<const storage::PointerCalculator>(scan.getResultTable());
  pointerCalculator->print();

  auto posList = *pointerCalculator->getPositions();
  std::sort(posList.begin(), posList.end());

  /*for (const auto& pos : posList)
    std::cout << pos << ", ";
  std::cout << std::endl;*/

  //auto hotTable = std::make_shared<storage::Table>(table->dictionaries();
  

  const size_t rowc = table->size();
  size_t curIndex = 0;
  for (size_t row = 0; row < rowc; ++row) {
    /*while (row > posList.at(curIndex)) {
      std::cout << "actually this should not occur" << std::endl;
      ++curIndex;
    }*/
    if (curIndex < posList.size() && row == posList.at(curIndex)) {
      std::cout << row << ": HOT" << std::endl;
      ++curIndex;
    }
    else {
      std::cout << row << ": COLD" << std::endl;
    }
  }
}

std::shared_ptr<PlanOperation> AgingRun::parse(const Json::Value &data) {
  std::shared_ptr<AgingRun> ar = std::make_shared<AgingRun>();

  if (!data.isMember("table"))
    throw std::runtime_error("A table must be specified for the AgingRun");
  ar->_tableName = data["table"].asString();

  //TODO maybe some more options

  return ar;
}

void AgingRun::setTable(const std::string& table) {
  _tableName = table;
}

} } // namespace hyrise::access

