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
#include "helper/make_unique.h"

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
  return loadParameterized(path, map);
}

void setParameterf(parameter_map_t& map, const std::string& name, float value) {
  map[name] = std::make_shared<FloatParameterValue>(value);
}

void setParameteri(parameter_map_t& map, const std::string& name, int value, size_t width) {
  map[name] = std::make_shared<IntParameterValue>(value, width);
}

void setParameters(parameter_map_t& map, const std::string& name, const std::string& value) {
  map[name] = std::make_shared<StringParameterValue>(value);
}

namespace {
  enum FormatType { IntFormatType, StringFormatType, FloatFormatType, NoneFormat};

  FormatType getType(char c) {
    switch (c) {
      case 'i':
      case 'd': return IntFormatType;
      case 'f': return FloatFormatType;
      case 's': return StringFormatType;
      default: return NoneFormat;
    }
  }
}

std::string loadParameterized(const std::string &path, const parameter_map_t& params) {
  std::ifstream data_file(path.c_str());
  std::string file((std::istreambuf_iterator<char>(data_file)), std::istreambuf_iterator<char>());
  data_file.close();

  for (auto& param : params) {
    size_t pos = (size_t) -1;
    //const std::string name = "%(" + param.first + ")";
    const std::string name = "%(" + param.first + ")";
    
    while ((pos = file.find(name, pos + 1)) != file.npos) {
      size_t curpos = pos + name.length();
      std::ostringstream os;

      if (isdigit(file.at(curpos))) {
        char* endptr;
        size_t width = strtol(file.c_str() + curpos, &endptr, 10);
	curpos = endptr - file.c_str();
	os << std::setw(width);
      }
      
      if (!isalpha(file.at(curpos)))
        throw std::runtime_error("no format set for parameter \'" + param.first + "\'");
      
      switch (getType(file.at(curpos))) {
        case FloatFormatType:
        case IntFormatType:    os << std::setfill('0'); break;
	case StringFormatType: os << std::setfill(' '); break;
	case NoneFormat: throw std::runtime_error("illegal format for parameter \'" + param.first + "\'");
      }

      os << param.second->toString();
      file.replace(pos, curpos + 1 - pos, os.str());
    }
  }

  return file;
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
    hyrise::tx::transaction_id_t* tid_ptr,
    size_t poolSize,
    std::string* evt) {
  using namespace hyrise;
  using namespace hyrise::access;
  using namespace hyrise::tx;
 
  std::stringstream query;
  query << "query=" << httpQuery;
  //if (tid_ptr == nullptr)
  //  query << "&autocommit=true";
  if (tid_ptr != nullptr && *tid_ptr != UNKNOWN) {
    const auto tid = *tid_ptr;
    query << "&session_context=" << tid;
  }

  std::unique_ptr<MockedConnection> conn = make_unique<MockedConnection>(query.str());

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
  
  if (tid_ptr != nullptr) {
    auto context = response->getTxContext();
    if (*tid_ptr == UNKNOWN)
      *tid_ptr = context.tid;
    else if (context.tid != *tid_ptr)
      throw std::runtime_error("requested transaction id ignored!");
  }

  return result_task->getResultTable();
}
