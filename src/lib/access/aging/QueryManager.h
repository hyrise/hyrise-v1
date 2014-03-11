// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <unordered_map>

#include <helper/types.h>
#include <storage/AgingIndex.h>
#include <storage/PointerCalculator.h>
#include <access/system/PlanOperation.h>
#include <access/aging/expressions/SelectExpression.h>

namespace hyrise {
namespace access {

class QueryManager {
public:
  QueryManager(const QueryManager&) = delete;

  static QueryManager& instance();

  void registerQuery(const std::string& name, std::shared_ptr<aging::SelectExpression> select);

  bool exists(const std::string& name) const;
  void assureExists(const std::string& name) const;

  query_t getId(const std::string& name) const;
  const std::string& getName(query_t query) const;

  std::shared_ptr<aging::SelectExpression> selectExpressionOf(query_t query) const;

  std::vector<query_t> queriesOfTable(storage::atable_ptr_t table);

  void clear();

private:
  QueryManager();

  void cleanRegistered();

  std::vector<std::shared_ptr<aging::SelectExpression>> _selectExpressions;
  std::unordered_map<std::string, query_t> _queryNames;
};

} } // namespace hyrise::access

