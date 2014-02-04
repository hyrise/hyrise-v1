// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "QueryManager.h"

#include <iostream>
#include <algorithm>

namespace hyrise {
namespace access {

QueryManager::QueryManager() {

}

QueryManager& QueryManager::instance() {
  static auto* qm = new QueryManager();
  return *qm;
}

void QueryManager::registerQuery(const std::string& name, std::unique_ptr<aging::SelectExpression>&& select) {
  if (exists(name))
    return; //TODO throw std::runtime_error("Query \"" + name + "\" already exists");
  select->verify();

  const query_t id = _queryParameters.size();
  _queryParameters.push_back(std::move(select));
  _queryNames.insert(std::make_pair(name, id));

  std::cout << "register Query \"" << name << "\"" << std::endl;

  cleanRegistered();

  /*TODO for (const auto& registeredIndex : _registered) {
    const auto& index = registeredIndex.lock();
    const auto tableId = index->table()->id();
    std::vector<field_t> fields;
    for (const auto& param : params) {
      if (param.table == tableId)
        fields.push_back(param.field);
    }
  }*/
}

bool QueryManager::exists(const std::string& name) const {
  return _queryNames.find(name) != _queryNames.end();
}

void QueryManager::assureExists(const std::string& name) const {
  if (!exists(name))
    throw std::runtime_error("Query \"" + name + "\" does not exist");
}

query_t QueryManager::getId(const std::string& name) const {
  const auto& it = _queryNames.find(name);
  if (it == _queryNames.end())
    assureExists(name); // which will fail, but no need to rewrite the error code
  return it->second;
}

const std::string& QueryManager::getName(query_t query) const {
  for (const auto& queryName : _queryNames) {
    if (queryName.second == query)
      return queryName.first;
  }
  throw std::runtime_error("QueryID is not registered");
}

aging::SelectExpression* QueryManager::selectExpressionOf(const std::string& query) const {
  return selectExpressionOf(getId(query));
}

aging::SelectExpression* QueryManager::selectExpressionOf(query_t query) const {
  if (query >= _queryParameters.size())
    throw std::runtime_error("invalid query id");
  return _queryParameters.at(query).get();
}

void QueryManager::registerAgingIndex(std::shared_ptr<storage::AgingIndex> index) {
  /*TODO const auto tableId = index->table()->id();
  std::vector<field_t> fields;

  for (query_t id = 0; id < _queryParameters.size(); ++id) {
    const auto& query = _queryParameters.at(id);
    for (const auto& param : query) {
      if (param.table == tableId)
        fields.push_back(param.field);
    }
  }*/

  _registered.push_back(index);
}

void QueryManager::cleanRegistered() {
  //TODO necessary
  _registered.erase(
    std::remove_if(_registered.begin(), _registered.end(),
        [](const std::weak_ptr<storage::AgingIndex>& index) { return index.expired(); }),
    _registered.end());
}

} } // namespace hyrise::access

