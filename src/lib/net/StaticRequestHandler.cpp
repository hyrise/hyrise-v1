// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "net/StaticRequestHandler.h"

#include <unordered_map>
#include <fstream>

#include "helper/fs.h"
#include "net/AsyncConnection.h"


#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"

#include "log4cxx/logger.h"

namespace hyrise { namespace net {

namespace { log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan.PlanOperation")); }

bool StaticRequestHandler::_registered = Router::registerRoute<StaticRequestHandler>("/static/");

StaticRequestHandler::StaticRequestHandler(net::AbstractConnection *connection): _connection(connection)
{
  // Whats the root path of the system
  char *data_path;
  data_path = getenv("HYRISE_STATIC_PATH");
  if (data_path != nullptr)
    _rootPath = std::string(data_path) + "/";
  else
    _rootPath = helper::sys_getcwd() + "/static/";
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
  result[""] = "application/binary";
  return result;
}

void StaticRequestHandler::operator()() {
  // Load all mime types once and for all
  static auto mimeTypes = parseMimeTypes();

  // Remove the static part of the URI
  std::string path(_connection->getPath().substr(8));

  const auto complete_path = _rootPath + path;
  if (!boost::filesystem::is_regular_file(complete_path)) {
    _connection->respond("404 - path is not a file: " + complete_path, 404);
    return;
  }

  // Check if File exists and load the content write to the conneciton buffer and exit
  std::ifstream infile(complete_path);
  if (infile.good()) {
    std::string contents { std::istreambuf_iterator<char>(infile),
                           std::istreambuf_iterator<char>() };
    const auto dotPos = path.rfind(".");
    _connection->respond(contents, 200, mimeTypes[path.substr(dotPos+1)]);
  } else {
    _connection->respond("404 - could not open file: " + complete_path, 404);
  }
}

}}
