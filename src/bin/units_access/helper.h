// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_BIN_UNITS_ACCESS_HELPER_H_
#define SRC_BIN_UNITS_ACCESS_HELPER_H_

#include <json.h>
#include <gtest/gtest.h>
#include "helper/HwlocHelper.h"
#include <storage/AbstractTable.h>

//  Joins specified columns of a single table using hash join.
hyrise::storage::c_atable_ptr_t hashJoinSameTable(
    hyrise::storage::atable_ptr_t table,
    field_list_t &columns);

//  Used for message chaining to improve code readability when building edges maps
class EdgesBuilder {
  typedef std::pair<std::string, std::string> edge_t;
  typedef std::vector<edge_t> edges_map_t;
  typedef std::vector< std::pair<std::string, std::string> >::const_iterator
  edges_map_iterator;
  edges_map_t edges;
 public:
  EdgesBuilder() : edges(edges_map_t()) {}
  ~EdgesBuilder() {}
  EdgesBuilder &clear();
  EdgesBuilder &appendEdge(const std::string &src, const std::string &dst);
  Json::Value getEdges() const;
};

hyrise::storage::c_atable_ptr_t sortTable(hyrise::storage::c_atable_ptr_t table);

bool isEdgeEqual(
    const Json::Value &edges,
    const unsigned position,
    const std::string &src,
    const std::string &dst);

std::string loadFromFile(std::string path);

hyrise::storage::c_atable_ptr_t executeAndWait(
    std::string httpQuery,
    size_t poolSize = getNumberOfCoresOnSystem(),
    std::string *evt = nullptr);

#endif  // SRC_BIN_UNITS_ACCESS_HELPER_H_
