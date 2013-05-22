// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "StaticRequestHandler.h"
#include "AsyncConnection.h"

#include <helper/fs.h>

#include <boost/algorithm/string.hpp>


#include <unordered_map>
#include <fstream>
#include <log4cxx/logger.h>

namespace hyrise { namespace net {

namespace { log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan._PlanOperation")); }

bool StaticRequestHandler::_registered = Router::registerRoute<StaticRequestHandler>("/static/");

StaticRequestHandler::StaticRequestHandler(net::AbstractConnection *connection): _connection(connection)
{
  // Whats the root path of the system
  char *data_path;
  data_path = getenv("HYRISE_STATIC_PATH");
  if (data_path != nullptr)
    _rootPath = std::string(data_path) + "/";
  else
    _rootPath = hyrise::helper::sys_getcwd() + "/static/";
}

std::unordered_map<std::string, std::string> parseMimeTypes() {
  std::unordered_map<std::string, std::string> result;
  std::ifstream infile("./third_party/mime/mime.types");
  if (infile.good()) {
    std::string buffer;
    while(infile.good()) {
      std::getline(infile, buffer);

      // Handle Comments
      if (buffer[0]=='#' || buffer.size() == 0)
        continue;

      // Split and parse the rest
      auto space_pos = buffer.find_first_of(" \t");
      auto mime_type = buffer.substr(0, space_pos);
      auto extensions = buffer.substr(space_pos);

      std::vector<std::string> allKinds;
      boost::algorithm::split(allKinds, extensions, boost::is_any_of(" "));
      boost::algorithm::trim(mime_type);

      for(const auto& t : allKinds) {
        // Now strip the types to get rid of extra spaces
        result[boost::algorithm::trim_copy(t)] = mime_type;

      }

    }
  }
  return result;
}

void StaticRequestHandler::operator()() {

  // Load all mime types once and for all
  static auto mimeTypes = parseMimeTypes();

  auto con = dynamic_cast<net::AsyncConnection*>(_connection);
  std::string path(con->path);

  // Remove the static part of the URI
  path = path.substr(8);

  LOG4CXX_DEBUG(logger, "Loading File: " << _rootPath << path);

  // Check if File exists and load the content write to the conneciton buffer and exit
  std::ifstream infile(_rootPath + path);
  if (infile.good()) {

    // Based on the extension set the mime type
    const auto dotPos = path.rfind(".");
    if (dotPos == std::string::npos)
      con->contentType = "application/binary";
    else
      con->contentType = mimeTypes[path.substr(dotPos+1)];

    std::string contents((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    con->respond(contents);
    return;
  }
  con->code = 404;
  con->respond("");
}

}}
