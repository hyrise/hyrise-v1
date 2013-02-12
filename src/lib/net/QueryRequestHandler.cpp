#include "QueryRequestHandler.h"

#include "AbstractConnection.h"

#include <access/RequestParseTask.h>


namespace {
	auto _ = hyrise::net::Router::registerRoute<hyrise::net::QueryRequestHandler>("/query/", hyrise::net::Router::route_t::CATCH_ALL);
}

namespace hyrise { namespace net {

QueryRequestHandler::QueryRequestHandler(net::AbstractConnection* connection)
    : _connection(connection) {

   _parseTask = new hyrise::access::RequestParseTask(connection);
}

void QueryRequestHandler::operator()() {
	(*_parseTask)();
}

QueryRequestHandler::~QueryRequestHandler() {
	delete _parseTask;
}

std::string QueryRequestHandler::name() {
  return "QueryRequestHandler";
}

const std::string QueryRequestHandler::vname() {
  return "QueryRequestHandler";
}

std::shared_ptr<hyrise::access::ResponseTask> QueryRequestHandler::getResponseTask() const {
  return _parseTask->getResponseTask();
}

}}