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

  void registerQuery(const std::string& name, std::unique_ptr<aging::SelectExpression>&& select);

  bool exists(const std::string& name) const;
  void assureExists(const std::string& name) const;
  query_t getId(const std::string& name) const;
  const std::string& getName(query_t query) const;

  aging::SelectExpression* selectExpressionOf(const std::string& query) const;
  aging::SelectExpression* selectExpressionOf(query_t query) const;

  //TODO necessary?
  void registerAgingIndex(std::shared_ptr<storage::AgingIndex> index);

  std::shared_ptr<storage::PointerCalculator> executeQuery(query_t query, param_list_t params);

private:
  QueryManager();

  void cleanRegistered();

  std::vector<std::unique_ptr<aging::SelectExpression>> _queryParameters;
  std::unordered_map<std::string, query_t> _queryNames;

  std::vector<std::weak_ptr<storage::AgingIndex>> _registered;
};

} } // namespace hyrise::access

