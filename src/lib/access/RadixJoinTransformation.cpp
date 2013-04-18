// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/RadixJoinTransformation.h"
#include "access/QueryTransformationEngine.h"

namespace hyrise {
namespace access {

bool RadixJoinTransformation::transformation_is_registered = QueryTransformationEngine::registerTransformation<RadixJoinTransformation>();

void RadixJoinTransformation::appendEdge(
    const std::string &srcId,
    const std::string &dstId,
    Json::Value &query) const {
  Json::Value edge(Json::arrayValue);
  edge.append(srcId);
  edge.append(dstId);
  query["edges"].append(edge);
}

Json::Value RadixJoinTransformation::createEdge(std::string in, std::string out){
  Json::Value edge(Json::arrayValue);
  edge.append(in);
  edge.append(out);
  return edge;
}

void RadixJoinTransformation::removeOperator(
    Json::Value &query,
    const Json::Value &operatorId) const {
  Json::Value remainingEdges(Json::arrayValue);
  Json::Value jsonOperatorId(operatorId);
  for (unsigned i = 0; i < query["edges"].size(); ++i) {
    Json::Value currentEdge = query["edges"][i];
    if (currentEdge[0u] != jsonOperatorId
        &&  currentEdge[1u] != jsonOperatorId) {
      remainingEdges.append(currentEdge);
    }
  }
  query["edges"] = remainingEdges;
  query["operators"].removeMember(operatorId.asString());
}

std::vector<std::string> RadixJoinTransformation::getInputIds(const std::string & id, const Json::Value &query){
  std::vector<std::string> inputs;
  Json::Value currentEdge;
  for (unsigned i = 0; i < query["edges"].size(); ++i) {
      currentEdge = query["edges"][i];
      if (currentEdge[1u] == id) {
        inputs.push_back(currentEdge[0u].asString());
      }
  }
  return inputs;
}

std::vector<std::string> RadixJoinTransformation::getOutputIds(const std::string & id, const  Json::Value &query){
  std::vector<std::string> outputs;
  Json::Value currentEdge;
  for (unsigned i = 0; i < query["edges"].size(); ++i) {
      currentEdge = query["edges"][i];
      if (currentEdge[0u] == id) {
        outputs.push_back(currentEdge[1u].asString());
      }
  }
  return outputs;
}

ops_and_edges_t RadixJoinTransformation::build_probe_side(std::string prefix,
                                                          Json::Value &fields,
                                                          int probe_par,
                                                          Json::Value & bits1,
                                                          Json::Value & bits2,
                                                          std::string in_id){
  ops_and_edges_t probe_side;

  Json::Value histogram_p1, prefixsum_p1, createradixtable_p1, radixcluster_p1, merge_prefix_sum, barrier;

  histogram_p1["type"] = "Histogram";
  histogram_p1["fields"] = fields;
  histogram_p1["bits"] = bits1;

  prefixsum_p1["type"] = "PrefixSum";

  createradixtable_p1["type"] = "CreateRadixTable";

  radixcluster_p1["type"] = "RadixCluster";
  radixcluster_p1["fields"] = fields;
  radixcluster_p1["bits"] = bits1;

  merge_prefix_sum["type"] = "MergePrefixSum";

  barrier["type"] = "Barrier";
  barrier["fields"] = fields;

  // First define the plan ops
  // add parallel ops
  for(int i = 0; i < probe_par; i++){
    Json::Value h = Json::Value(histogram_p1);
    h["part"] = i;
    h["numParts"] = probe_par;
    Json::Value p = Json::Value(prefixsum_p1);
    p["part"] = i;
    p["numParts"] = probe_par;
    Json::Value r = Json::Value(radixcluster_p1);
    r["part"] = i;
    r["numParts"] = probe_par;
    probe_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_histogram_p1_" + std::to_string(i), h));
    probe_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_prefixsum_p1_" + std::to_string(i), p));
    probe_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_radixcluster_p1_" + std::to_string(i), r));
  }
  // add seriable ops
  probe_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_createradixtable_p1", createradixtable_p1));
  probe_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_merge_prefixsum", merge_prefix_sum));
  probe_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_barrier", barrier));

  // Then define the edges

  // There is an edge from input to create cluster table
  probe_side.edges.push_back(createEdge(in_id, prefix + "_createradixtable_p1"));

  for(int i = 0; i < probe_par; i++){

    //the input goes to all histograms
    probe_side.edges.push_back(createEdge(in_id, prefix + "_histogram_p1_" + std::to_string(i)));

    //All equal histograms go to all prefix sums
    for(int j = 0; j < probe_par; j++){
      probe_side.edges.push_back(createEdge(prefix + "_histogram_p1_" + std::to_string(i), prefix + "_prefixsum_p1_" + std::to_string(j)));
    }

    // And from each input there is a link to radix clustering
    probe_side.edges.push_back(createEdge(in_id, prefix + "_radixcluster_p1_" + std::to_string(i)));

    // From create radix table to radix cluster for the second pass as well
    probe_side.edges.push_back(createEdge(prefix + "_createradixtable_p1", prefix + "_radixcluster_p1_" + std::to_string(i)));

    // From each prefix sum there is a link to radix clustering
    probe_side.edges.push_back(createEdge(prefix + "_prefixsum_p1_" + std::to_string(i), prefix + "_radixcluster_p1_" + std::to_string(i)));

    //  Merge all prefix sums
    probe_side.edges.push_back(createEdge(prefix + "_prefixsum_p1_" + std::to_string(i), prefix + "_merge_prefixsum"));
    probe_side.edges.push_back(createEdge(prefix + "_radixcluster_p1_" + std::to_string(i), prefix + "_barrier"));
  }

  return probe_side;
}

ops_and_edges_t RadixJoinTransformation::build_hash_side(std::string prefix,
                                                         Json::Value &fields,
                                                         int hash_par,
                                                         Json::Value & bits1,
                                                         Json::Value & bits2,
                                                         std::string in_id){
  ops_and_edges_t hash_side;

  Json::Value histogram_p1, prefixsum_p1, createradixtable_p1, radixcluster_p1, histogram_p2, prefixsum_p2, createradixtable_p2, radixcluster_p2, merge_prefix_sum, barrier;

  histogram_p1["type"] = "Histogram";
  histogram_p1["fields"] = fields;
  histogram_p1["bits"] = bits1;

  prefixsum_p1["type"] = "PrefixSum";

  createradixtable_p1["type"] = "CreateRadixTable";

  radixcluster_p1["type"] = "RadixCluster";
  radixcluster_p1["fields"] = fields;
  radixcluster_p1["bits"] = bits1;


  histogram_p2["type"] = "Histogram2ndPass";
  histogram_p2["bits"] = bits1;
  histogram_p2["bits2"] = bits2;
  histogram_p2["sig2"] = bits1;

  prefixsum_p2["type"] = "PrefixSum";

  createradixtable_p2["type"] = "CreateRadixTable";

  radixcluster_p2["type"] = "RadixCluster2ndPass";
  radixcluster_p2["bits"] = bits1;
  radixcluster_p2["bits2"] = bits2;
  radixcluster_p2["sig2"] = bits1;

  merge_prefix_sum["type"] = "MergePrefixSum";

  barrier["type"] = "Barrier";
  barrier["fields"] = fields;

  // add parallel ops
  for(int i = 0; i < hash_par; i++){
    Json::Value h = Json::Value(histogram_p1);
    h["part"] = i;
    h["numParts"] = hash_par;
    Json::Value p = Json::Value(prefixsum_p1);
    p["part"] = i;
    p["numParts"] = hash_par;
    Json::Value r = Json::Value(radixcluster_p1);
    r["part"] = i;
    r["numParts"] = hash_par;
    Json::Value h2 = Json::Value(histogram_p2);
    h2["part"] = i;
    h2["numParts"] = hash_par;
    Json::Value p2 = Json::Value(prefixsum_p2);
    p2["part"] = i;
    p2["numParts"] = hash_par;
    Json::Value r2 = Json::Value(radixcluster_p2);
    r2["part"] = i;
    r2["numParts"] = hash_par;
    hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_histogram_p1_" + std::to_string(i), h));
    hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_prefixsum_p1_" + std::to_string(i), p));
    hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_radixcluster_p1_" + std::to_string(i), r));
    hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_histogram_p2_" + std::to_string(i), h2));
    hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_prefixsum_p2_" + std::to_string(i), p2));
    hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_radixcluster_p2_" + std::to_string(i), r2));
  }
  // add seriable ops
  hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_createradixtable_p1", createradixtable_p1));
  hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_createradixtable_p2", createradixtable_p2));
  hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_merge_prefixsum", merge_prefix_sum));
  hash_side.ops.push_back(std::pair<std::string, Json::Value>(prefix + "_barrier", barrier));

  // Then define the edges
    // There is an edge from input to create cluster table and for the second pass
  hash_side.edges.push_back(createEdge(in_id, prefix + "_createradixtable_p1"));
  hash_side.edges.push_back(createEdge(in_id, prefix + "_createradixtable_p2"));

  for(int i = 0; i < hash_par; i++){

    //the input goes to all histograms
    hash_side.edges.push_back(createEdge(in_id, prefix + "_histogram_p1_" + std::to_string(i)));

    //All equal histograms go to all prefix sums
    for(int j = 0; j < hash_par; j++){
      hash_side.edges.push_back(createEdge(prefix + "_histogram_p1_" + std::to_string(i), prefix + "_prefixsum_p1_" + std::to_string(j)));
    }

    // And from each input there is a link to radix clustering
    hash_side.edges.push_back(createEdge(in_id, prefix + "_radixcluster_p1_" + std::to_string(i)));

    // From create radix table to radix cluster for the second pass as well
    hash_side.edges.push_back(createEdge(prefix + "_createradixtable_p1", prefix + "_radixcluster_p1_" + std::to_string(i)));

    // From each prefix sum there is a link to radix clustering
    hash_side.edges.push_back(createEdge(prefix + "_prefixsum_p1_" + std::to_string(i), prefix + "_radixcluster_p1_" + std::to_string(i)));

    // now comes the second pass which is like the first only a litte
    // more complicated
    for(int j = 0; j < hash_par; j++){
       //We need an explicit barrier here to avoid that a histogram is calculated before all other
       //first pass radix clusters finished
      hash_side.edges.push_back(createEdge(prefix + "_radixcluster_p1_" + std::to_string(i), prefix + "_histogram_p2_" + std::to_string(j)));
      hash_side.edges.push_back(createEdge(prefix + "_histogram_p2_" + std::to_string(j), prefix + "_prefixsum_p2_" + std::to_string(i)));
    }
    // This builds up the second pass radix cluster, attention order matters
    hash_side.edges.push_back(createEdge(prefix + "_radixcluster_p1_" + std::to_string(i), prefix + "_radixcluster_p2_" + std::to_string(i)));
    hash_side.edges.push_back(createEdge(prefix + "_createradixtable_p2", prefix + "_radixcluster_p2_" + std::to_string(i)));
    hash_side.edges.push_back(createEdge(prefix + "_prefixsum_p2_" + std::to_string(i), prefix + "_radixcluster_p2_" + std::to_string(i)));
    hash_side.edges.push_back(createEdge(prefix + "_prefixsum_p2_" + std::to_string(i), prefix + "_merge_prefixsum"));
    hash_side.edges.push_back(createEdge(prefix + "_radixcluster_p2_" + std::to_string(i), prefix + "_barrier"));
  }
  return hash_side;
}

void RadixJoinTransformation::distributePartitions(
				  const int partitions,
				  const int join_count,
				  const int current_join,
				  int &first,
				  int &last) const {
  const int
    partitionsPerJoin     = partitions / join_count,
    remainingElements   = partitions - partitionsPerJoin * join_count,
    extraElements       = current_join <= remainingElements ? current_join : remainingElements,
    partsExtraElement   = current_join < remainingElements ? 1 : 0;
  first                   = partitionsPerJoin * current_join + extraElements;
  last                    = first + partitionsPerJoin + partsExtraElement - 1;
}

void RadixJoinTransformation::transform(Json::Value &op, const std::string &operatorId, Json::Value &query){
  int probe_par = op["probe_par"].asInt();
  int hash_par = op["hash_par"].asInt();
  int join_par = op["join_par"].asInt();
  Json::Value bits1 = op["bits1"];
  Json::Value bits2 = op["bits2"];
  Json::Value fields = op["fields"];

  Json::Value probe_field, hash_field;
  probe_field.append(fields[0]);
  hash_field.append(fields[1]);

  std::vector<std::string> input_edges = getInputIds(operatorId, query);
  std::vector<std::string> output_edges = getOutputIds(operatorId, query);

  // remove op and all edges
  removeOperator(query, operatorId);

  // create ops and edges for probe side
  ops_and_edges_t probe_side = build_probe_side(operatorId + "_probe", probe_field, probe_par, bits1, bits2, input_edges[0]);
  // add ops from probe side to query
  for(size_t i = 0; i < probe_side.ops.size(); i++)
    query["operators"][probe_side.ops.at(i).first] = probe_side.ops.at(i).second;
  // add edges from probe side to query
  for(size_t i = 0; i < probe_side.edges.size(); i++)
    query["edges"].append(probe_side.edges.at(i));

  // create ops and edges for hash side
  ops_and_edges_t hash_side = build_hash_side(operatorId + "_hash", hash_field, hash_par, bits1, bits2, input_edges[1]);
  // add ops from hash side to query
  for(size_t i = 0; i < hash_side.ops.size(); i++)
    query["operators"][hash_side.ops.at(i).first] = hash_side.ops.at(i).second;
  // add edges from hash side to query
  for(size_t i = 0; i < hash_side.edges.size(); i++)
    query["edges"].append(hash_side.edges.at(i));

  // We have as many partitions as we bits in the first pass have
  int partitions = 1 << bits1.asInt();

  // create join ops
  int first = 0, last = 0;
  std::string join_name;
  // indexes into hash_ops and probe_ops to connect edges to join ops
  int probe_barrier = probe_side.ops.size() - 1;
  int probe_prefix = probe_side.ops.size() - 2;
  int hash_barrier = hash_side.ops.size() - 1;
  int hash_prefix = hash_side.ops.size() - 2;

  // case 1: no parallel join -> we do not need a union and have to connect the output edges to join
  if(join_par == 1){
    join_name = operatorId + "_join";

    Json::Value join(Json::objectValue);
    join["type"] = "NestedLoopEquiJoin";
    join["bits1"] = op["bits1"];
    join["bits2"] = op["bits2"];
    Json::Value parts;
    for(int i = 0; i < partitions; i++){
      parts.append(i);
    }
    join["partitions"] = parts;
    query["operators"][join_name] = join;
    for(size_t i = 0; i < output_edges.size(); i++)
      appendEdge(join_name, output_edges[i], query);
    // create edges
    // probe input table
    appendEdge(input_edges[0], join_name, query);
    // probe barrier
    appendEdge(probe_side.ops.at(probe_side.ops.size()-1).first, join_name, query);
    // probe merge prefix sum
    appendEdge(probe_side.ops.at(probe_side.ops.size()-2).first, join_name, query);
    // hash input table
    appendEdge(input_edges[1], join_name, query);
    // hash barrier
    appendEdge(hash_side.ops.at(hash_side.ops.size()-1).first, join_name, query);
    // hash merge prefix sum
    appendEdge(hash_side.ops.at(hash_side.ops.size()-2).first, join_name, query);
  } else {
    // case 2: parallel join -> we do need a union and have to connect the output edges to union

    Json::Value unionOperator(Json::objectValue);
    unionOperator["type"] = "UnionAll";
    std::string unionId = operatorId + "_union";
    query["operators"][unionId] = unionOperator;
    // build output edges for union
    for(size_t i = 0; i < output_edges.size(); i++)
      appendEdge(unionId, output_edges[i], query);

    // calculate partitions that need to be worked by join
    // if join_par > partitions, set join_par to partitions
    if(join_par > partitions)
      join_par = partitions;
    for(int i = 0; i < join_par;  i++){
      join_name = operatorId + "_join_" + std::to_string(i);/*
      first = (partitions / join_par) * i;
      if(i + 1 < join_par)
        last = ((partitions / join_par) * (i + 1)) - 1;
      else
      last = partitions - 1;*/
      distributePartitions(partitions, join_par, i, first, last);
      // create join

      Json::Value join(Json::objectValue);
      join["type"] = "NestedLoopEquiJoin";
      join["bits1"] = op["bits1"];
      join["bits2"] = op["bits2"];
      Json::Value partitions;
      for(int i = first; i <= last; i++){
        partitions.append(i);
      }
      join["partitions"] = partitions;
      query["operators"][join_name] = join;

      std::string unionId = operatorId + "_union";
      // create edges
      // probe input table
      appendEdge(input_edges[0], join_name, query);
      // probe barrier
      appendEdge(probe_side.ops.at(probe_barrier).first, join_name, query);
      // probe merge prefix sum
      appendEdge(probe_side.ops.at(probe_prefix).first, join_name, query);
      // hash input table
      appendEdge(input_edges[1], join_name, query);
      // hash barrier
      appendEdge(hash_side.ops.at(hash_barrier).first, join_name, query);
      // hash merge prefix sum
      appendEdge(hash_side.ops.at(hash_prefix).first, join_name, query);
      // out union
      appendEdge(join_name, unionId, query);
    }
  }
}

}
}
