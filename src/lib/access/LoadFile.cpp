#include "access/LoadFile.h"

#include <stdexcept>

#include "access/QueryParser.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"

static auto registered = QueryParser::registerPlanOperation<LoadFile>("LoadFile");

LoadFile::LoadFile(const std::string& filename) :
    _filename(filename) {}

std::shared_ptr<_PlanOperation> LoadFile::parse(Json::Value& data) {
  if (data["filename"].asString().empty())
    throw std::runtime_error("LoadFile invalid without \"filename\": ...");
  return std::make_shared<LoadFile>(data["filename"].asString());
}

const std::string LoadFile::vname() {
  return "LoadFile";
}

void LoadFile::executePlanOperation() {
  output.add(Loader::shortcuts::load(StorageManager::getInstance()->makePath(_filename)));
}

