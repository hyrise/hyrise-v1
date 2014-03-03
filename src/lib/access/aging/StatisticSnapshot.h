// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <unordered_map>

#include <storage/AbstractStatistic.h>

namespace hyrise {
namespace access {

typedef std::vector<bool> hotness_vector_t;
typedef std::unordered_map<query_t, hotness_vector_t> hotness_map_t;

class StatisticSnapshot : public storage::AbstractStatistic {
 public:
  explicit StatisticSnapshot(const storage::AbstractStatistic& other);
  virtual ~StatisticSnapshot() {}

  virtual bool isHot(query_t query, storage::value_id_t vid) const;
  virtual bool isQueryRegistered(query_t query) const;
  bool isValueRegistered(storage::value_id_t vid) const;

  virtual void valuesDo(query_t query, std::function<void(storage::value_id_t, bool)> func) const;

  virtual std::vector<query_t> queries() const;
  virtual std::vector<storage::value_id_t> vids() const;

 private:
  size_t numberOfVids() const;

  const hotness_map_t _hotnessMap;
};

} } // namespace hyrise::access

