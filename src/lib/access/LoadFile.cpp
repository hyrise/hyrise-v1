// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/LoadFile.h"

#include <stdexcept>

#include "access/QueryParser.h"

#include "io/shortcuts.h"
#include "io/StorageManager.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<LoadFile>("LoadFile");
}

LoadFile::LoadFile(const std::string &filename) : _filename(filename) {
}

LoadFile::~LoadFile() {
}

void LoadFile::executePlanOperation() {
  output.add(Loader::shortcuts::load(StorageManager::getInstance()->makePath(_filename)));
}

std::shared_ptr<_PlanOperation> LoadFile::parse(Json::Value &data) {
  if (data["filename"].asString().empty())
    throw std::runtime_error("LoadFile invalid without \"filename\": ...");
  return std::make_shared<LoadFile>(data["filename"].asString());
}

const std::string LoadFile::vname() {
  return "LoadFile";
}

}
}
