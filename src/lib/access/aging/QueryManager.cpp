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

void QueryManager::registerQuery(const std::string& name, const param_list_t& params) {
  if (exists(name))
    return; //TODO throw std::runtime_error("Query \"" + name + "\" already exists");

  const query_t id = _queryParameters.size();
  _queryParameters.push_back(params);
  _queryNames.insert(std::make_pair(name, id));

  std::cout << "register Query \"" << name << "\"" << std::endl;

  cleanRegistered();

  for (const auto& registeredIndex : _registered) {
    const auto& index = registeredIndex.lock();
    const auto tableId = index->table()->id();
    std::vector<field_t> fields;
    for (const auto& param : params) {
      if (param.table == tableId)
        fields.push_back(param.field);
    }
    if (fields.size() > 0)
      index->addForQuery(id, fields);
  }
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

param_list_t QueryManager::parametersOf(const std::string& query) const {
  return parametersOf(getId(query));
}

param_list_t QueryManager::parametersOf(query_t query) const {
  if (query >= _queryParameters.size())
    throw std::runtime_error("invalid query id");
  return _queryParameters.at(query);
}

void QueryManager::registerAgingIndex(std::shared_ptr<storage::AgingIndex> index) {
  const auto tableId = index->table()->id();
  std::vector<field_t> fields;

  for (query_t id = 0; id < _queryParameters.size(); ++id) {
    const auto& query = _queryParameters.at(id);
    for (const auto& param : query) {
      if (param.table == tableId)
        fields.push_back(param.field);
    }
    if (fields.size() > 0)
      index->addForQuery(id, fields);
  }

  _registered.push_back(index);
}

void QueryManager::cleanRegistered() {
  _registered.erase(
    std::remove_if(_registered.begin(), _registered.end(),
        [](const std::weak_ptr<storage::AgingIndex>& index) { return index.expired(); }),
    _registered.end());
}

} } // namespace hyrise::access

