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

void QueryManager::registerQuery(const std::string& name, const param_vector_t& params) {
  if (exists(name))
    return; //TODO throw std::runtime_error("Query \"" + name + "\" already exists");

  const query_id_t id = _queryParameters.size();
  _queryParameters.push_back(params);
  _queryNames.insert(std::make_pair(name, id));

  std::cout << "registeres Query \"" << name << "\"" << std::endl;
}

bool QueryManager::exists(const std::string& name) const {
  return _queryNames.find(name) == _queryNames.end();
}

void QueryManager::assureExists(const std::string& name) const {
  if (!exists(name))
    throw std::runtime_error("Query \"" + name + "\" does not exist");
}

query_id_t QueryManager::getId(const std::string& name) const {
  const auto& it = _queryNames.find(name);
  if (it == _queryNames.end())
    assureExists(name); // which will fail, but no need to rewrite the error code
  return it->second;
}

param_vector_t QueryManager::parametersOf(const std::string& query) const {
  return parametersOf(getId(query));
}

param_vector_t QueryManager::parametersOf(query_id_t query) const {
  if (query >= _queryParameters.size())
    throw std::runtime_error("invalid query id");
  return _queryParameters.at(query);
}

void QueryManager::registerAgingIndex(std::shared_ptr<storage::AgingIndex> index) {
  
}

void QueryManager::cleanRegistered() {
  _registered.erase(
    std::remove_if(_registered.begin(), _registered.end(),
        [](const std::weak_ptr<storage::AgingIndex>& index) { return index.expired(); }),
    _registered.end());
}

} } // namespace hyrise::access

