// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "RadixJoinTransformation.h"
#include "access/system/QueryTransformationEngine.h"

namespace hyrise {
namespace access {

bool RadixJoinTransformation::transformation_is_registered = QueryTransformationEngine::registerTransformation<RadixJoinTransformation>();

void RadixJoinTransformation::appendEdge(
    const std::string &srcId,
    const std::string &dstId,
    rapidjson::Document &query) const {

  rapidjson::Value edge(rapidjson::kArrayType);
  edge.PushBack(srcId, query.GetAllocator());
  edge.PushBack(dstId, query.GetAllocator());
  query["edges"].PushBack(edge, query.GetAllocator());
}

rapidjson::Document RadixJoinTransformation::createEdge(rapidjson::Document& q, std::string in, std::string out){
  rapidjson::Document edge;
  edge.SetArray();
  edge.PushBack(in, edge.GetAllocator());
  edge.PushBack(out, edge.GetAllocator());
  return std::move(edge);
}

void RadixJoinTransformation::removeOperator(
    rapidjson::Document &query,
    const rapidjson::Value &operatorId) const {

  rapidjson::Value remainingEdges(rapidjson::kArrayType);

  for (unsigned i = 0; i < query["edges"].size(); ++i) {
    
    // Copy the edge
    rapidjson::Document currentEdge;
    query["edges"][i].Accept(currentEdge);

    if (currentEdge[0u].asString() != operatorId.asString()
        &&  currentEdge[1u].asString() != operatorId.asString()) {
      remainingEdges.PushBack(currentEdge, query.GetAllocator());
    }
  }
  
  query.AddMember("edges", remainingEdges, query.GetAllocator());
  query["operators"].RemoveMember(operatorId.asString().c_str());
}

std::vector<std::string> RadixJoinTransformation::getInputIds(const std::string & id, const rapidjson::Document &query){
  std::vector<std::string> inputs;

  for (unsigned i = 0; i < query["edges"].size(); ++i) {
    const rapidjson::Value& currentEdge = query["edges"][i];
    if (currentEdge[1u].asString() == id) {
      inputs.push_back(currentEdge[0u].asString());
    }
  }
  return inputs;
}

std::vector<std::string> RadixJoinTransformation::getOutputIds(const std::string & id, const  rapidjson::Document &query){
  std::vector<std::string> outputs;
  for (unsigned i = 0; i < query["edges"].size(); ++i) {
    const rapidjson::Value& currentEdge = query["edges"][i];
    if (currentEdge[0u].asString() == id) {
      outputs.push_back(currentEdge[1u].asString());
    }
  }
  return outputs;
}

ops_and_edges_t RadixJoinTransformation::build_probe_side( rapidjson::Document& query,
                                                           std::string prefix,
                                                           rapidjson::Value &fields,
                                                           int probe_par,
                                                           const rapidjson::Value & bits1,
                                                           const rapidjson::Value & bits2,
                                                           std::string in_id){
  ops_and_edges_t probe_side;

  // Creating lots of elements, could be slow
  rapidjson::Document histogram_p1, prefixsum_p1, createradixtable_p1, radixcluster_p1, merge_prefix_sum, barrier;
  
  histogram_p1.SetObject();
  histogram_p1.AddMember("type", "Histogram", histogram_p1.GetAllocator());
  histogram_p1.AddMember("fields", fields, histogram_p1.GetAllocator());
  histogram_p1.AddMember("bits", bits1.asUInt(), histogram_p1.GetAllocator());

  prefixsum_p1.SetObject();
  prefixsum_p1.AddMember("type", "PrefixSum", prefixsum_p1.GetAllocator());

  createradixtable_p1.SetObject();
  createradixtable_p1.AddMember("type", "CreateRadixTable", createradixtable_p1.GetAllocator());

  radixcluster_p1.SetObject();
  radixcluster_p1.AddMember("type", "RadixCluster", radixcluster_p1.GetAllocator());
  radixcluster_p1.AddMember("fields", fields, radixcluster_p1.GetAllocator());
  radixcluster_p1.AddMember("bits", bits1.asUInt(), radixcluster_p1.GetAllocator());

  merge_prefix_sum.SetObject();
  merge_prefix_sum.AddMember("type", "MergePrefixSum", merge_prefix_sum.GetAllocator());

  barrier.SetObject();
  barrier.AddMember("type", "Barrier", barrier.GetAllocator());
  barrier.AddMember("fields", fields, barrier.GetAllocator());

  // First define the plan ops
  // add parallel ops
  for(int i = 0; i < probe_par; i++){

    rapidjson::Document h;
    histogram_p1.Accept(h);
    h.AddMember("part", i, h.GetAllocator());
    h.AddMember("numParts", probe_par, h.GetAllocator());

    rapidjson::Document p;
    prefixsum_p1.Accept(p);
    p.AddMember("part", i, p.GetAllocator());
    p.AddMember("numParts", probe_par, p.GetAllocator());

    
    rapidjson::Document r;
    radixcluster_p1.Accept(r);
    r.AddMember("part", i, r.GetAllocator());
    r.AddMember("numParts", probe_par, r.GetAllocator());

    probe_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_histogram_p1_" + std::to_string(i), h));
    probe_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_prefixsum_p1_" + std::to_string(i), p));
    probe_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_radixcluster_p1_" + std::to_string(i), r));
  }
  // add seriable ops
  probe_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_createradixtable_p1", createradixtable_p1));
  probe_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_merge_prefixsum", merge_prefix_sum));
  probe_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_barrier", barrier));

  // Then define the edges

  // There is an edge from input to create cluster table
  probe_side.edges.push_back(createEdge(query, in_id, prefix + "_createradixtable_p1"));

  for(int i = 0; i < probe_par; i++){

    //the input goes to all histograms
    probe_side.edges.push_back(createEdge(query, in_id, prefix + "_histogram_p1_" + std::to_string(i)));

    //All equal histograms go to all prefix sums
    for(int j = 0; j < probe_par; j++){
      probe_side.edges.push_back(createEdge(query, prefix + "_histogram_p1_" + std::to_string(i), prefix + "_prefixsum_p1_" + std::to_string(j)));
    }

    // And from each input there is a link to radix clustering
    probe_side.edges.push_back(createEdge(query, in_id, prefix + "_radixcluster_p1_" + std::to_string(i)));

    // From create radix table to radix cluster for the second pass as well
    probe_side.edges.push_back(createEdge(query, prefix + "_createradixtable_p1", prefix + "_radixcluster_p1_" + std::to_string(i)));

    // From each prefix sum there is a link to radix clustering
    probe_side.edges.push_back(createEdge(query, prefix + "_prefixsum_p1_" + std::to_string(i), prefix + "_radixcluster_p1_" + std::to_string(i)));

    //  Merge all prefix sums
    probe_side.edges.push_back(createEdge(query, prefix + "_prefixsum_p1_" + std::to_string(i), prefix + "_merge_prefixsum"));
    probe_side.edges.push_back(createEdge(query, prefix + "_radixcluster_p1_" + std::to_string(i), prefix + "_barrier"));
  }

  return probe_side;
}

ops_and_edges_t RadixJoinTransformation::build_hash_side(
    rapidjson::Document& query, 
    std::string prefix,
    rapidjson::Value &fields,
    int hash_par,
    const rapidjson::Value & bits1,
    const rapidjson::Value & bits2,
    std::string in_id){
  ops_and_edges_t hash_side;

  rapidjson::Document histogram_p1, prefixsum_p1, createradixtable_p1, 
      radixcluster_p1, histogram_p2, prefixsum_p2, createradixtable_p2, 
      radixcluster_p2, merge_prefix_sum, barrier;

  histogram_p1.SetObject();
  histogram_p1.AddMember("type", "Histogram", histogram_p1.GetAllocator());
  histogram_p1.AddMember("fields", fields, histogram_p1.GetAllocator());
  histogram_p1.AddMember("bits", bits1.asUInt(), histogram_p1.GetAllocator());

  prefixsum_p1.SetObject();
  prefixsum_p1.AddMember("type", "PrefixSum", prefixsum_p1.GetAllocator());

  createradixtable_p1.SetObject();
  createradixtable_p1.AddMember("type", "CreateRadixTable", createradixtable_p1.GetAllocator());

  radixcluster_p1.SetObject();
  radixcluster_p1.AddMember("type", "RadixCluster", radixcluster_p1.GetAllocator());
  radixcluster_p1.AddMember("fields", fields, radixcluster_p1.GetAllocator());
  radixcluster_p1.AddMember("bits", bits1.asUInt(), radixcluster_p1.GetAllocator());

  histogram_p2.SetObject();
  histogram_p2.AddMember("type", "Histogram2ndPass", histogram_p2.GetAllocator());
  histogram_p2.AddMember("bits", bits1.asUInt(), histogram_p2.GetAllocator());
  histogram_p2.AddMember("bits2", bits2.asUInt(), histogram_p2.GetAllocator());
  histogram_p2.AddMember("sig2", bits1.asUInt(), histogram_p2.GetAllocator());

  prefixsum_p2.SetObject();
  prefixsum_p2.AddMember("type", "PrefixSum", prefixsum_p2.GetAllocator());

  createradixtable_p2.SetObject();
  createradixtable_p2.AddMember("type", "CreateRadixTable", createradixtable_p2.GetAllocator());

  radixcluster_p2.SetObject();
  radixcluster_p2.AddMember("type", "RadixCluster2ndPass", radixcluster_p2.GetAllocator());
  radixcluster_p2.AddMember("bits", bits1.asUInt(), radixcluster_p2.GetAllocator());
  radixcluster_p2.AddMember("bits2", bits2.asUInt(), radixcluster_p2.GetAllocator());
  radixcluster_p2.AddMember("sig2", bits1.asUInt(), radixcluster_p2.GetAllocator());

  merge_prefix_sum.SetObject();
  merge_prefix_sum.AddMember("type", "MergePrefixSum", merge_prefix_sum.GetAllocator());

  barrier.SetObject();
  barrier.AddMember("type", "Barrier", barrier.GetAllocator());
  barrier.AddMember("fields", fields, barrier.GetAllocator());


  // add parallel ops
  for(int i = 0; i < hash_par; i++){
    rapidjson::Document h;
    histogram_p1.Accept(h);
    h.AddMember("part", i, h.GetAllocator());
    h.AddMember("numParts", hash_par, h.GetAllocator());
    
    rapidjson::Document p;
    prefixsum_p1.Accept(p);
    p.AddMember("part", i, p.GetAllocator());
    p.AddMember("numParts", hash_par, p.GetAllocator());

    rapidjson::Document r;
    radixcluster_p1.Accept(r);
    r.AddMember("part", i, r.GetAllocator());
    r.AddMember("numParts", hash_par, r.GetAllocator());

    rapidjson::Document h2;
    histogram_p2.Accept(h2);
    h2.AddMember("part", i, h2.GetAllocator());
    h2.AddMember("numParts", hash_par, h2.GetAllocator());
    
    rapidjson::Document p2;
    prefixsum_p2.Accept(p2);
    p2.AddMember("part", i, p2.GetAllocator());
    p2.AddMember("numParts", hash_par, p2.GetAllocator());

    rapidjson::Document r2;
    radixcluster_p2.Accept(r2);
    r2.AddMember("part", i, r2.GetAllocator());
    r2.AddMember("numParts", hash_par, r2.GetAllocator());

    hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_histogram_p1_" + std::to_string(i), h));
    hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_prefixsum_p1_" + std::to_string(i), p));
    hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_radixcluster_p1_" + std::to_string(i), r));
    hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_histogram_p2_" + std::to_string(i), h2));
    hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_prefixsum_p2_" + std::to_string(i), p2));
    hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_radixcluster_p2_" + std::to_string(i), r2));
  }
  // add seriable ops
  hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_createradixtable_p1", createradixtable_p1));
  hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_createradixtable_p2", createradixtable_p2));
  hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_merge_prefixsum", merge_prefix_sum));
  hash_side.ops.push_back(std::pair<std::string, rapidjson::Document>(prefix + "_barrier", barrier));

  // Then define the edges
    // There is an edge from input to create cluster table and for the second pass
  hash_side.edges.push_back(createEdge(query, in_id, prefix + "_createradixtable_p1"));
  hash_side.edges.push_back(createEdge(query, in_id, prefix + "_createradixtable_p2"));

  for(int i = 0; i < hash_par; i++){

    //the input goes to all histograms
    hash_side.edges.push_back(createEdge(query, in_id, prefix + "_histogram_p1_" + std::to_string(i)));

    //All equal histograms go to all prefix sums
    for(int j = 0; j < hash_par; j++){
      hash_side.edges.push_back(createEdge(query, prefix + "_histogram_p1_" + std::to_string(i), prefix + "_prefixsum_p1_" + std::to_string(j)));
    }

    // And from each input there is a link to radix clustering
    hash_side.edges.push_back(createEdge(query, in_id, prefix + "_radixcluster_p1_" + std::to_string(i)));

    // From create radix table to radix cluster for the second pass as well
    hash_side.edges.push_back(createEdge(query, prefix + "_createradixtable_p1", prefix + "_radixcluster_p1_" + std::to_string(i)));

    // From each prefix sum there is a link to radix clustering
    hash_side.edges.push_back(createEdge(query, prefix + "_prefixsum_p1_" + std::to_string(i), prefix + "_radixcluster_p1_" + std::to_string(i)));

    // now comes the second pass which is like the first only a litte
    // more complicated
    for(int j = 0; j < hash_par; j++){
       //We need an explicit barrier here to avoid that a histogram is calculated before all other
       //first pass radix clusters finished
      hash_side.edges.push_back(createEdge(query, prefix + "_radixcluster_p1_" + std::to_string(i), prefix + "_histogram_p2_" + std::to_string(j)));
      hash_side.edges.push_back(createEdge(query, prefix + "_histogram_p2_" + std::to_string(j), prefix + "_prefixsum_p2_" + std::to_string(i)));
    }
    // This builds up the second pass radix cluster, attention order matters
    hash_side.edges.push_back(createEdge(query, prefix + "_radixcluster_p1_" + std::to_string(i), prefix + "_radixcluster_p2_" + std::to_string(i)));
    hash_side.edges.push_back(createEdge(query, prefix + "_createradixtable_p2", prefix + "_radixcluster_p2_" + std::to_string(i)));
    hash_side.edges.push_back(createEdge(query, prefix + "_prefixsum_p2_" + std::to_string(i), prefix + "_radixcluster_p2_" + std::to_string(i)));
    hash_side.edges.push_back(createEdge(query, prefix + "_prefixsum_p2_" + std::to_string(i), prefix + "_merge_prefixsum"));
    hash_side.edges.push_back(createEdge(query, prefix + "_radixcluster_p2_" + std::to_string(i), prefix + "_barrier"));
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

void RadixJoinTransformation::transform(const rapidjson::Value &op, const std::string &operatorId, rapidjson::Document &query){
  int probe_par = op["probe_par"].asInt();
  int hash_par = op["hash_par"].asInt();
  int join_par = op["join_par"].asInt();
  const rapidjson::Value& bits1 = op["bits1"];
  const rapidjson::Value& bits2 = op["bits2"];
  const rapidjson::Value& fields = op["fields"];

  rapidjson::Value probe_field(rapidjson::kArrayType), hash_field(rapidjson::kArrayType);
  probe_field.PushBack(fields[0u].asUInt(), query.GetAllocator());
  hash_field.PushBack(fields[1u].asUInt(), query.GetAllocator());

  std::vector<std::string> input_edges = getInputIds(operatorId, query);
  std::vector<std::string> output_edges = getOutputIds(operatorId, query);

  // remove op and all edges
  removeOperator(query, operatorId);

  // create ops and edges for probe side
  ops_and_edges_t probe_side = build_probe_side(query, operatorId + "_probe", probe_field, probe_par, bits1, bits2, input_edges[0]);
  // add ops from probe side to query
  for(size_t i = 0; i < probe_side.ops.size(); i++)
    query["operators"][probe_side.ops.at(i).first] = probe_side.ops.at(i).second;
  // add edges from probe side to query
  for(size_t i = 0; i < probe_side.edges.size(); i++)
    query["edges"].PushBack(probe_side.edges.at(i), query.GetAllocator());

  // create ops and edges for hash side
  ops_and_edges_t hash_side = build_hash_side(query, operatorId + "_hash", hash_field, hash_par, bits1, bits2, input_edges[1]);
  // add ops from hash side to query
  for(size_t i = 0; i < hash_side.ops.size(); i++)
    query["operators"][hash_side.ops.at(i).first] = hash_side.ops.at(i).second;
  // add edges from hash side to query
  for(size_t i = 0; i < hash_side.edges.size(); i++)
    query["edges"].PushBack(hash_side.edges.at(i), query.GetAllocator());

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

    rapidjson::Value join(rapidjson::kObjectType);
    join.AddMember("type", "NestedLoopEquiJoin", query.GetAllocator());
    join.AddMember("bits1", op["bits1"].asUInt(), query.GetAllocator());
    join.AddMember("bits2", op["bits2"].asUInt(), query.GetAllocator());
    
    rapidjson::Value parts(rapidjson::kArrayType);
    for(int i = 0; i < partitions; i++){
      parts.PushBack(i, query.GetAllocator());
    }
    join.AddMember("partitions", parts, query.GetAllocator());
    query["operators"].AddMember(join_name.c_str(), join, query.GetAllocator());

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

    rapidjson::Value unionOperator(rapidjson::kObjectType);
    unionOperator.AddMember("type", "UnionAll", query.GetAllocator());

    std::string unionId = operatorId + "_union";
    query["operators"].AddMember(unionId.c_str(), unionOperator, query.GetAllocator());;
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

      rapidjson::Value join(rapidjson::kObjectType);
      join.AddMember("type", "NestedLoopEquiJoin", query.GetAllocator());
      join.AddMember("bits1", op["bits1"].asUInt(), query.GetAllocator());
      join.AddMember("bits2", op["bits2"].asUInt(), query.GetAllocator());

      rapidjson::Value partitions(rapidjson::kArrayType);
      for(int i = first; i <= last; i++){
        partitions.PushBack(i, query.GetAllocator());
      }
      join.AddMember("partitions", partitions, query.GetAllocator());
      query["operators"].AddMember(join_name.c_str(), join, query.GetAllocator());

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
