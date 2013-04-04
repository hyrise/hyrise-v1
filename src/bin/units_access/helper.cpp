// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "helper.h"

#include <iostream>
#include <fstream>
#include <memory>

#include "helper/HttpHelper.h"
#include "helper/types.h"

#include <testing/test.h>

#include <access.h>
#include <io.h>
#include <storage.h>
#include <taskscheduler.h>


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
	hashJoinProbe->setFields(&columns);
	hashJoinProbe->addInputHash(hashBuild->execute()->getResultHashTable());

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




std::string loadFromFile(std::string path) {
  std::ifstream data_file(path.c_str());
  std::string result((std::istreambuf_iterator<char>(data_file)), std::istreambuf_iterator<char>());
  data_file.close();
  return result;
}

#include "net/AbstractConnection.h"
#include "access/RequestParseTask.h"
#include "access/PlanOperation.h"
#include "access/ResponseTask.h"

class MockedConnection : public hyrise::net::AbstractConnection {
 public:
  MockedConnection(const std::string& body) : _body(body) {}

  virtual void respond(const std::string& r) {
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
    std::string *evt) {
  using namespace hyrise;
  std::unique_ptr<MockedConnection> conn(new MockedConnection("query="+httpQuery));

  if(!SharedScheduler::getInstance().isInitialized())
    SharedScheduler::getInstance().init("WSSimpleTaskScheduler");
  AbstractTaskScheduler * scheduler = SharedScheduler::getInstance().getScheduler();
  scheduler->resize(poolSize);

  auto request = std::make_shared<access::RequestParseTask>(conn.get());
  auto response = request->getResponseTask();

  auto wait = std::make_shared<WaitTask>();
  wait->addDependency(response);

  scheduler->schedule(wait);
  scheduler->schedule(request);

  wait->wait();

  auto result_task = response->getResultTask();
  if (result_task == nullptr) {
    throw std::runtime_error("Response:" + conn->getResponse());
  }

  if (evt != nullptr) {
    *evt = result_task->getEvent();
  }

  if (result_task->getState() == OpFail) {
    std::string msg = result_task->getErrorMessage();
    throw std::runtime_error(msg);
  }

  return result_task->getResultTable();
}
