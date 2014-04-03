// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <json.h>
#include <gtest/gtest.h>

#include <helper/HwlocHelper.h>
#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <io/TXContext.h>

namespace hyrise {
namespace access {

//  Joins specified columns of a single table using hash join.
storage::c_atable_ptr_t hashJoinSameTable(storage::atable_ptr_t table, field_list_t& columns);

//  Used for message chaining to improve code readability when building edges maps
class EdgesBuilder {
  typedef std::pair<std::string, std::string> edge_t;
  typedef std::vector<edge_t> edges_map_t;
  typedef std::vector<std::pair<std::string, std::string> >::const_iterator edges_map_iterator;
  edges_map_t edges;

 public:
  EdgesBuilder() : edges(edges_map_t()) {}
  ~EdgesBuilder() {}
  EdgesBuilder& clear();
  EdgesBuilder& appendEdge(const std::string& src, const std::string& dst);
  Json::Value getEdges() const;
};

storage::c_atable_ptr_t sortTable(storage::c_atable_ptr_t table);

bool isEdgeEqual(const Json::Value& edges, const unsigned position, const std::string& src, const std::string& dst);

class ParameterValue;
typedef std::shared_ptr<ParameterValue> value_ptr_t;
typedef std::map<std::string, value_ptr_t> parameter_map_t;

class ParameterValue {
 public:
  virtual std::string toString() const = 0;
};

class FloatParameterValue : public ParameterValue {
 public:
  FloatParameterValue(float value) : _value(value) {}

  virtual std::string toString() const {
    std::ostringstream os;
    os << _value;
    return os.str();
  }

 protected:
  const float _value;
};

class IntParameterValue : public ParameterValue {
 public:
  IntParameterValue(int value, size_t width = 0) : _value(value) {}

  virtual std::string toString() const {
    std::ostringstream os;
    os << _value;
    return os.str();
  }

 protected:
  const int _value;
};

class StringParameterValue : public ParameterValue {
 public:
  StringParameterValue(const std::string& value) : _value(value) {}

  virtual std::string toString() const { return '\"' + _value + '\"'; }

 protected:
  const std::string _value;
};

void setParameter(parameter_map_t& map, std::string name, int value);
void setParameter(parameter_map_t& map, std::string name, float value);
void setParameter(parameter_map_t& map, std::string name, std::string value);

std::string loadParameterized(const std::string& path, const parameter_map_t& params);

tx::TXContext getNewTXContext();

std::string loadFromFile(const std::string& path);

storage::c_atable_ptr_t executeAndWait(std::string httpQuery,
                                       size_t poolSize = getNumberOfCoresOnSystem(),
                                       std::string* evt = nullptr,
                                       tx::transaction_id_t tid = tx::UNKNOWN);

std::string executeStoredProcedureAndWait(std::string storedProcedureName,
                                          std::string httpQuery,
                                          size_t poolSize = getNumberOfCoresOnSystem());
}
}  // namespace hyrise::access
