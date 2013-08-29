// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "helper.h"

#include <iostream>
#include <fstream>
#include <memory>

#include "access/HashBuild.h"
#include "access/HashJoinProbe.h"
#include "access/system/RequestParseTask.h"
#include "access/system/ResponseTask.h"
#include "access/SortScan.h"

#include "helper/HttpHelper.h"
#include "helper/types.h"

#include "net/AbstractConnection.h"

#include "storage/AbstractTable.h"
#include "storage/AbstractHashTable.h"

#include "taskscheduler/SharedScheduler.h"

#include "testing/test.h"

hyrise::storage::c_atable_ptr_t sortTable(hyrise::storage::c_atable_ptr_t table){
  size_t c = table->columnCount();
  for(size_t f = 0; f < c; f++){
    hyrise::access::SortScan so;
    so.addInput(table);
    so.setSortField(f);
    table = so.execute()->getResultTable();
  }
  return table;
}
const hyrise::storage::c_atable_ptr_t hashJoinSameTable(

    hyrise::storage::c_atable_ptr_t& table,
    field_list_t &columns) {

	/*
  std::vector<std::shared_ptr<hyrise::access::HashBuild> > hashBuilds;
  for (unsigned int i = 0; i < columns.size(); ++i) {
    hashBuilds.push_back(std::make_shared<hyrise::access::HashBuild>());
    hashBuilds[i]->addInput(table);
    hashBuilds[i]->addField(columns[i]);
  }
	 */
	auto hashBuild = std::make_shared<hyrise::access::HashBuild>();
	hashBuild->addInput(table);
	hashBuild->setKey("join");
	for (unsigned int i = 0; i < columns.size(); ++i) {
		hashBuild->addField(columns[i]);
	}
	auto hashJoinProbe = std::make_shared<hyrise::access::HashJoinProbe>();
	hashJoinProbe->addInput(table);
        for (auto col: columns) { hashJoinProbe->addField(col); }
	hashJoinProbe->addInput(hashBuild->execute()->getResultHashTable());

	return hashJoinProbe->execute()->getResultTable();
}

EdgesBuilder &EdgesBuilder::clear() {
	edges.clear();
	return *this;
}

EdgesBuilder &EdgesBuilder::appendEdge(
		const std::string &src,
		const std::string &dst) {
	const edge_t newEdge = edge_t(src, dst);
	edges.push_back(newEdge);
	return *this;
}

Json::Value EdgesBuilder::getEdges() const {
	Json::Value jsonEdges = Json::Value(Json::arrayValue);
	for (edges_map_iterator it = this->edges.begin(); it != this->edges.end(); ++it) {
		Json::Value srcNode(it->first);
    Json::Value dstNode(it->second);
    Json::Value edge(Json::arrayValue);
    edge.append(srcNode);
    edge.append(dstNode);
    jsonEdges.append(edge);
  }
  return jsonEdges;
}


bool isEdgeEqual(
    const Json::Value &edges,
    const unsigned position,
    const std::string &src,
    const std::string &dst) {
  Json::Value currentEdge = edges[position];
  return currentEdge[0u] == src && currentEdge[1u] == dst;
}




std::string loadFromFile(const std::string& path) {
  parameter_map_t map;
  return loadParameterized(path, map, false);
}

template <>
void addParameter<hyrise_float_t>(parameter_map_t& map, const std::string& name,
                                  hyrise_float_t value) {
  map[name] = std::make_shared<FloatParameterValue>(value);
}

template <>
void addParameter<hyrise_int_t>(parameter_map_t& map, const std::string& name,
                                  hyrise_int_t value) {
  map[name] = std::make_shared<IntParameterValue>(value);
}

template <>
void addParameter<hyrise_string_t>(parameter_map_t& map, const std::string& name,
                                  hyrise_string_t value) {
  map[name] = std::make_shared<StringParameterValue>(value);
}

std::string loadParameterized(const std::string &path, const parameter_map_t& params,
                                 bool exceptionIfUnknownParameter) {
  std::ifstream data_file(path.c_str());
  std::string file((std::istreambuf_iterator<char>(data_file)), std::istreambuf_iterator<char>());
  data_file.close();

  std::ostringstream os;
  char lastChar = 0;
  bool lineComment = false; // '//[...]'
  bool comment = false; //     '/*[...]*/
  bool inString = false; //    '"[...]"
  size_t parameterStart = 0;
  size_t currentPos = 0;
  for (char c : file) {
    if (lineComment) {
      if (c == '\n') {
        lineComment = false;
        os << c;
      } 
    }
    else if (comment) {
      if (lastChar == '*' && c == '/') {
        comment = false;
        lastChar = c = 0;
      }
    }
    else if (inString) {
      if (c == '\"')
        inString = false;
        os << c;
	lastChar = c = 0;
      }
    else if (parameterStart != 0) {
      if (c == '>') {
        const std::string parameterName = file.substr(parameterStart, currentPos - parameterStart);
        //os << "!PARAMETER(" << parameterName << ")!";
        if (params.find(parameterName) != params.end())
	  os << params.at(parameterName)->toString();
	else if (exceptionIfUnknownParameter)
	  throw std::runtime_error("Parameter \"" + parameterName + "\" not specified in parameter map");
        parameterStart = 0;
	lastChar = c = 0;
      }
    }
    else if (c == '\"') {
      inString = true;
      os << c;
    }
    else if (c == '/') {
      if (lastChar == '/')
        lineComment = true;
    }
    else if (c == '*') {
      if (lastChar == '/')
        comment = true;
      else
        os << c;
    }
    else if (c == '<') {
      parameterStart = currentPos + 1;
    }
    else {
      if (lastChar == '/')
        os << lastChar;
      os << c;
    }
    lastChar = c;
    ++currentPos;
  }
  return os.str();
}

class MockedConnection : public hyrise::net::AbstractConnection {
 public:
  MockedConnection(const std::string& body) : _body(body) {}

  virtual void respond(const std::string& r, std::size_t code, const std::string& contentType) {
    _response = r;
  }

  std::string getResponse() {
    return _response;
  }

  bool hasBody() const {
    return !_body.empty();
  }

  std::string getBody() const {
    return _body;
  }

  std::string getPath() const {
    return "";
  }
 private:
  std::string _body;
  std::string _response;
};

/**
 * This function is used to simulate the execution of plan operations
 * using the threadpool. The input to this function is a JSON std::string
 * that will be parsed and the necessary plan operations will be
 * instantiated.
 */
hyrise::storage::c_atable_ptr_t executeAndWait(
    std::string httpQuery,
    size_t poolSize,
    std::string* evt) {
  using namespace hyrise;
  using namespace hyrise::access;
  std::unique_ptr<MockedConnection> conn(new MockedConnection("query="+httpQuery));

  SharedScheduler::getInstance().resetScheduler("WSCoreBoundQueuesScheduler", poolSize);
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();

  auto request = std::make_shared<access::RequestParseTask>(conn.get());
  auto response = request->getResponseTask();

  auto wait = std::make_shared<WaitTask>();
  wait->addDependency(response);

  scheduler->schedule(wait);
  scheduler->schedule(request);

  wait->wait();


  
  auto result_task = response->getResultTask();
  
  /*
  */
  
  if (response->getState() == OpFail) {
    throw std::runtime_error(joinString(response->getErrorMessages(), "\n"));
  }

  if (result_task == nullptr) {
    throw std::runtime_error("Response: " + conn->getResponse());
  }

  if (evt != nullptr) {
    *evt = result_task->getEvent();
  }
  
  return result_task->getResultTable();
}
