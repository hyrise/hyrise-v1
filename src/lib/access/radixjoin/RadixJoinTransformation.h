// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H
#define SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H

#include "access/RadixJoin.h"
#include <json.h>
#include "access/system/AbstractPlanOpTransformation.h"

namespace hyrise {
namespace access {

typedef struct ops_and_edges {
  std::vector<std::pair<std::string, Json::Value>> ops;
  std::vector<Json::Value> edges;
} ops_and_edges_t;

/*
 * This class transforms a virtual operator RadixJoin into a set of operators that perform the radix join.
 * The transformation is based on a Json query that is rewritten; the actual operators are instatiated at a later stage.
 * Currently only sequential plans are supported!!!
 */
class RadixJoinTransformation : public AbstractPlanOpTransformation {
  static bool transformation_is_registered;

  Json::Value createEdge(std::string in, std::string out);
  void appendEdge(const std::string& srcId, const std::string& dstId, Json::Value& query) const;
  void removeOperator(Json::Value& query, const Json::Value& operatorId) const;
  std::vector<std::string> getInputIds(const std::string& id, const Json::Value& query);
  std::vector<std::string> getOutputIds(const std::string& id, const Json::Value& query);
  ops_and_edges_t build_hash_side(std::string prefix,
                                  Json::Value& fields,
                                  int hash_par,
                                  Json::Value& bits1,
                                  Json::Value& bits2,
                                  std::string in_id);
  ops_and_edges_t build_probe_side(std::string prefix,
                                   Json::Value& fields,
                                   int probe_par,
                                   Json::Value& bits1,
                                   Json::Value& bits2,
                                   std::string in_id);
  void distributePartitions(const int partitions, const int join_count, const int current_join, int& first, int& last)
      const;

 public:
  RadixJoinTransformation() {};
  virtual ~RadixJoinTransformation() {};

  void transform(Json::Value& op, const std::string& operatorId, Json::Value& query);

  static const std::string name() { return "RadixJoin"; }
};
}
}

#endif  // SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H
