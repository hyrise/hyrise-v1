// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PipelineUnion.h"
#include "log4cxx/logger.h"

#include "access/UnionAll.h"
#include "access/system/OperationData-Impl.h"

#include "storage/AbstractTable.h"
#include "storage/HorizontalTable.h"

#include "helper/types.h"
#include "helper/vector_helpers.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerTrivialPlanOperation<PipelineUnion>("PipelineUnion");
log4cxx::LoggerPtr _pipelineLogger(log4cxx::Logger::getLogger("pipelining.PipelineUnion"));
}


// merges all input tables or input hash tables with the current output and clears the input.
void PipelineUnion::executePlanOperation() {
  LOG4CXX_DEBUG(_pipelineLogger, "executePlanOperation()");
  auto tables = _pipelineInput([](OperationData & pipelineInput)->std::vector<storage::c_atable_ptr_t> {
    return pipelineInput.allOf<storage::AbstractTable>();
  });
  addResult(unionTables(tables));
}


storage::c_atable_ptr_t PipelineUnion::unionTables(std::vector<storage::c_atable_ptr_t> tables) {
  LOG4CXX_DEBUG(_pipelineLogger, "Unioning " << tables.size() << " tables.");
  return std::make_shared<const storage::HorizontalTable>(tables);
}

void PipelineUnion::notifyNewChunk(storage::c_aresource_ptr_t chunk) {
  LOG4CXX_DEBUG(_pipelineLogger, "Got new table chunk.");
  _pipelineInput([&chunk](OperationData& pipelineInput) { pipelineInput.addResource(chunk); });
}
}
}
