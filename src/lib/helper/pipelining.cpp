#include "helper/pipelining.h"

#include <sstream>

#include <boost/uuid/uuid.hpp>  // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators
#include <boost/uuid/uuid_io.hpp>  // streaming operators etc.

std::string getChunkIdentifier(std::string prefix) {
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::stringstream id;
  id << prefix << "_" << uuid;
  return id.str();
};
