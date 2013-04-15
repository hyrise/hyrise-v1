// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LoadWithDefaultDict.h"

#include <stdexcept>

#include "access/QueryParser.h"

#include "io/shortcuts.h"
#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<LoadWithDefaultDict>("LoadWithDefaultDict");
}

LoadWithDefaultDict::LoadWithDefaultDict(
    const std::string &filename, const bool forceMutable) : 
  _filename(filename),
  _forceMutable(forceMutable) {
}

LoadWithDefaultDict::~LoadWithDefaultDict() {
}

void LoadWithDefaultDict::executePlanOperation() {
  Loader::params p;
  p.setisDefaultDictVector(true);
  p.setReturnsMutableVerticalTable(_forceMutable);
  p.setModifiableMutableVerticalTable(_forceMutable);
  output.add(Loader::shortcuts::load(
        StorageManager::getInstance()->makePath(_filename),
        p));
}

std::shared_ptr<_PlanOperation> LoadWithDefaultDict::parse(Json::Value &data) {
  if (data["filename"].asString().empty())
    throw std::runtime_error("LoadWithDefaultDict invalid without \"filename\": ...");
  bool forceMutable = false;
  if (!data["forceMutable"].asString().empty()) {
    forceMutable = (data["forceMutable"].asString() == "true");
  }
  return std::make_shared<LoadWithDefaultDict>(data["filename"].asString(), forceMutable);
}

const std::string LoadWithDefaultDict::vname() {
  return "LoadWithDefaultDict";
}

}
}
