// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/RadixJoinTransformation.h"
#include "access/QueryTransformationEngine.h"

namespace hyrise {
namespace access {

bool RadixJoinTransformation::is_registered = QueryTransformationEngine::registerTransformation<RadixJoinTransformation>();

void RadixJoinTransformation::appendEdge(
    const std::string &srcId,
    const std::string &dstId,
    Json::Value &query) const {
  Json::Value edge(Json::arrayValue);
  edge.append(srcId);
  edge.append(dstId);
  query["edges"].append(edge);
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

std::vector<Json::Value> RadixJoinTransformation::createFirstPass(const Json::Value &radix){
  Json::Value histogramm, prefixsum, createradixtable, radixcluster;

  std::vector<Json::Value> retVal;

  histogramm["type"] = "Histogram";
  histogramm["fields"] = radix["fields"];
  histogramm["bits"] = radix["bits1"];

  prefixsum["type"] = "PrefixSum";

  createradixtable["type"] = "CreateRadixTable";

  radixcluster["type"] = "RadixCluster";
  radixcluster["fields"] = radix["fields"];
  radixcluster["bits"] = radix["bits1"];

  retVal.push_back(histogramm);
  retVal.push_back(prefixsum);
  retVal.push_back(createradixtable);
  retVal.push_back(radixcluster);

  return retVal;
}

std::vector<Json::Value> RadixJoinTransformation::createSecondPass(const Json::Value &radix){
  Json::Value histogramm, prefixsum, radixcluster;

  std::vector<Json::Value> retVal;

  histogramm["type"] = "Histogram2ndPass";
  histogramm["fields"] = radix["fields"];
  histogramm["bits"] = radix["bits2"];
  histogramm["sig"] = radix["bits1"];

  prefixsum["type"] = "PrefixSum";

  radixcluster["type"] = "RadixCluster2ndPass";
  radixcluster["fields"] = radix["fields"];
  radixcluster["bits"] = radix["bits2"];
  radixcluster["sig"] = radix["bits1"];

  retVal.push_back(histogramm);
  retVal.push_back(prefixsum);
  retVal.push_back(radixcluster);

  return retVal;
}

Json::Value RadixJoinTransformation::createJoin(const Json::Value &radix){
  Json::Value join;
  join["type"] = "NestedLoopEquiJoin";
  join["fields"] = radix["fields"];
  join["bits1"] = radix["bits1"];
  join["bits2"] = radix["bits2"];

  // 2^bits1 partitions
  int partitions = 1;
  for (int i=0; i<radix["bits1"].asInt(); i++) 
    partitions=partitions*2;
  for(int i = 0; i < partitions; i++){
  	join["partitions"].append(i);
  }
  return join;
}

void RadixJoinTransformation::connectFirstPassWithInput(std::vector<std::string> &pass, std::string input, Json::Value &query){
  // connect histogram 
  appendEdge(input, pass.at(0), query);
  // connect create radix table
  appendEdge(input, pass.at(2), query);
  // connect radix cluster
  appendEdge(input, pass.at(3), query);
}


void RadixJoinTransformation::connectFirstPass(std::vector<std::string> &pass, Json::Value &query){
  // connect prefix sum
  appendEdge(pass.at(0), pass.at(1), query);
  // connect create radix table
  appendEdge(pass.at(1), pass.at(2), query);
  // connect radix cluster
  appendEdge(pass.at(2), pass.at(3), query);
}

void RadixJoinTransformation::connectSecondPass(std::vector<std::string> &pass, Json::Value &query){
  // connect prefix sum
  appendEdge(pass.at(0), pass.at(1), query);
  // connect radix cluster
  appendEdge(pass.at(1), pass.at(2), query);
}

void RadixJoinTransformation::connectFirstAndSecondPass(std::vector<std::string> &firstpass, std::vector<std::string> &secondpass, Json::Value &query){
  // connect 2nd pass histogramm with first pass prefix and radix cluster
  // first radix cluster
  appendEdge(firstpass.at(3), secondpass.at(0), query);
  // second prefix
  appendEdge(firstpass.at(1), secondpass.at(0), query);
  appendEdge(firstpass.at(3), secondpass.at(2), query);

}

void RadixJoinTransformation::connectFirstPassWithJoin(std::vector<std::string> &pass, std::string join, Json::Value &query){
  // connect radix cluster
  appendEdge(pass.at(3), join, query);
    // connect prefix sum table
  appendEdge(pass.at(1), join, query);
}

void RadixJoinTransformation::connectSecondPassWithJoin(std::vector<std::string> &pass, std::string join, Json::Value &query){
  // connect radix cluster
  appendEdge(pass.at(2), join, query);
    // connect prefix sum table
  appendEdge(pass.at(1), join, query);
}

void RadixJoinTransformation::connectJoinWithInput(std::string input, std::string join, Json::Value &query){
  appendEdge(input, join, query);
}

void RadixJoinTransformation::transform(Json::Value &op, const std::string &operatorId, Json::Value &query){
  // get inputs and outputs of join; radix join expects two inputs
  std::string left_input, right_input;
  std::vector<std::string> outputs;
  bool foundFirst = false;
  for (unsigned i = 0; i < query["edges"].size(); ++i) {
    Json::Value currentEdge = query["edges"][i];
    if (currentEdge[1u] == operatorId) {
  		if(!foundFirst){
        foundFirst = true;
  			left_input = currentEdge[0u].asString();
  		}
      else{
  			right_input = currentEdge[0u].asString();
  			break;
  		}
    }
    else if (currentEdge[0u] == operatorId) {
      outputs.push_back(currentEdge[1u].asString());
    }
	}
  	

  // create first_pass for left side
  std::vector<std::string> leftFirstPassIds {"lhistogram", "lprefixsum", "lcreateradixtable", "lradixcluster"};
  std::vector<Json::Value> leftFirstPass = createFirstPass(op);
  for(size_t i = 0; i < leftFirstPass.size(); i++)
  	query["operators"][leftFirstPassIds.at(i)] = leftFirstPass.at(i);
  
  // create first_pass for right side
  std::vector<std::string> rightFirstPassIds {"rhistogram", "rprefixsum", "rcreateradixtable", "rradixcluster"};
  std::vector<Json::Value> rightFirstPass = createFirstPass(op);
  for(size_t i = 0; i < rightFirstPass.size(); i++)
  	query["operators"][rightFirstPassIds.at(i)] = rightFirstPass.at(i);
  
  // create second_pass for right side, if bits2 >0
  std::vector<Json::Value> rightSecondPass;
  std::vector<std::string> rightSecondPassIds {"rhistogram2", "rprefixsum2", "rradixcluster2"};
  if(op["bits2"].asInt() > 0){
    rightSecondPass = createSecondPass(op);
    for(size_t i = 0; i < rightSecondPass.size(); i++)
  		query["operators"][rightSecondPassIds.at(i)] = rightSecondPass.at(i);
  }
  
  // create join
  std::string joinId = "nestedloopequijoin";
  Json::Value join = createJoin(op);
  query["operators"][joinId] = join;
  
  // connect left side
      // connect first pass with input
  connectFirstPassWithInput(leftFirstPassIds, left_input, query);
  	// connect first pass
  connectFirstPass(leftFirstPassIds, query);
    // connect 
  connectJoinWithInput(left_input, joinId, query);	
  	// connect first pass with join
  connectFirstPassWithJoin(leftFirstPassIds, joinId, query);

    // connect right side
      // connect first pass with input
  connectFirstPassWithInput(rightFirstPassIds, right_input, query);
  	// connect first pass
  connectFirstPass(rightFirstPassIds, query);
    // connect 
  connectJoinWithInput(right_input, joinId, query);	
  	// connect pass with join
  if(op["bits2"].asInt() > 0){
    // first inputs of first pass
    connectFirstAndSecondPass(rightFirstPassIds, rightSecondPassIds, query);
    connectSecondPass(rightSecondPassIds, query);
  	connectSecondPassWithJoin(rightSecondPassIds, joinId, query);
  }
  else
  	connectFirstPassWithJoin(rightFirstPassIds, joinId, query);
  
  // connect join with oputputs
  for(size_t i = 0; i < outputs.size(); i++)
      appendEdge(joinId, outputs.at(0), query);

  // remove RadixJoin
  removeOperator(query, operatorId);
} 

}
}