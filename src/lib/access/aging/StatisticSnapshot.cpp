// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "StatisticSnapshot.h"

#include <access/aging/QueryManager.h>

namespace hyrise {
namespace access {

namespace {
hotness_map_t copyFrom(const storage::AbstractStatistic& stat) {
  hotness_map_t ret;
  const auto& queries = stat.queries();
  const size_t vidc = stat.vids().size();
  for (const auto& query : queries) {
    auto it = ret.insert(std::make_pair(query, hotness_vector_t(vidc)));
    auto& hotnessVector = it.first->second;
    stat.valuesDo(query, [&hotnessVector](storage::value_id_t vid, bool hot){ hotnessVector.at(vid) = hot; });
  }
  return ret;
}

} // namespace

StatisticSnapshot::StatisticSnapshot(const storage::AbstractStatistic& other) :
    AbstractStatistic(other),
    _hotnessMap(copyFrom(other)) {}

bool StatisticSnapshot::isHot(query_t query, storage::value_id_t vid) const {
  const auto& hotnessVector = _hotnessMap.find(query);
  if (hotnessVector == _hotnessMap.cend())
    return false;

  if (vid >= numberOfVids())
    return false; //false to be sure ... however, in aging pocesses true will usually be (more) correct
  
  return hotnessVector->second.at(vid);
}

bool StatisticSnapshot::isQueryRegistered(query_t query) const {
  return _hotnessMap.find(query) != _hotnessMap.cend();
}

bool StatisticSnapshot::isVidRegistered(storage::value_id_t vid) const {
  return (vid < numberOfVids());
}

void StatisticSnapshot::valuesDo(query_t query, std::function<void(storage::value_id_t, bool)> func) const {
  if (!isQueryRegistered(query)) {
    const auto& qm = QueryManager::instance();
    if (!qm.exists(qm.getName(query)))
      throw std::runtime_error("QueryID does not exists");
    return;
  }
  const auto& hotnessVector = _hotnessMap.at(query);
  for (size_t i = 0; i < hotnessVector.size(); ++i)
    func(i, hotnessVector.at(i));
}

std::vector<query_t> StatisticSnapshot::queries() const {
  std::vector<query_t> ret(_hotnessMap.size());
  size_t cur = 0;
  for (const auto& query : _hotnessMap)
    ret[cur++] = query.first;
  return ret;
}

std::vector<storage::value_id_t> StatisticSnapshot::vids() const {
  const size_t vidc = numberOfVids();
  std::vector<storage::value_id_t> ret(vidc);
  for (size_t i = 0; i < vidc; ++i)
    ret[i] = i;
  return ret;
}

size_t StatisticSnapshot::numberOfVids() const {
  return (_hotnessMap.size() == 0) ? 0 : (_hotnessMap.begin()->second.size());
}

} } // namespace hyrise::access

