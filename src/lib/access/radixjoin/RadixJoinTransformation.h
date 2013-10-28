// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H
#define SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H

#include "access/RadixJoin.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "access/system/AbstractPlanOpTransformation.h"

namespace hyrise {
namespace access {

typedef struct ops_and_edges{
  std::vector<std::pair<std::string, rapidjson::Document>> ops;
  std::vector<rapidjson::Document> edges;
} ops_and_edges_t;

 /*
  * This class transforms a virtual operator RadixJoin into a set of operators that perform the radix join.
  * The transformation is based on a Json query that is rewritten; the actual operators are instatiated at a later stage.
  * Currently only sequential plans are supported!!!
  */
class RadixJoinTransformation: public AbstractPlanOpTransformation {
  static bool transformation_is_registered;
  
  rapidjson::Document createEdge(rapidjson::Document& d, std::string in, std::string out);
  void appendEdge(const std::string &srcId,const std::string &dstId,rapidjson::Document &query) const;

  void removeOperator(rapidjson::Document &query,const rapidjson::Value &operatorId) const;

  std::vector<std::string> getInputIds(const std::string &id, const rapidjson::Document &query);

  std::vector<std::string> getOutputIds(const std::string &id, const rapidjson::Document &query);

  ops_and_edges_t build_hash_side(rapidjson::Document& doc, std::string prefix, rapidjson::Value &fields, int hash_par, const rapidjson::Value & bits1, const rapidjson::Value & bits2, std::string in_id);

  ops_and_edges_t build_probe_side(rapidjson::Document& doc, std::string prefix, rapidjson::Value &fields, int probe_par, const rapidjson::Value & bits1, const rapidjson::Value & bits2, std::string in_id);

  void distributePartitions(const int partitions, const int join_count, const int current_join, int &first, int &last) const;

public:
  RadixJoinTransformation(){};

  virtual ~RadixJoinTransformation(){};

  void transform(const rapidjson::Value &op, const std::string &operatorId, rapidjson::Document &query);

  static const std::string name() {
    return "RadixJoin";
  }

};

}
}

#endif // SRC_LIB_ACCES_RADIXJOINTRANSFORMATION_H
