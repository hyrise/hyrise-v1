// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PipelineStream.h"
#include "helper/types.h"
#include "storage/TableRangeView.h"
#include "log4cxx/logger.h"
#include <algorithm>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<PipelineStream>("PipelineStream");
log4cxx::LoggerPtr _pipelineLogger(log4cxx::Logger::getLogger("pipelining.PipelineStream"));
}

void PipelineStream::executePlanOperation() {
  // A predecessor will always exist
  auto dep = getFirstPredecessorOf<PlanOperation>();
  auto inputTable = dep->getResultTable();
  if (!inputTable) {
    throw std::runtime_error("Need input for stream chunking!");
  }

  size_t inputSize = inputTable->size();

  LOG4CXX_DEBUG(_pipelineLogger, "Blocking input found. Begin chunking.");

  auto numChunks = inputSize / _chunkSize + 1;

  for (size_t i = 0; i < numChunks; ++i) {
    size_t start = i * _chunkSize;
    size_t end = std::min((i + 1) * _chunkSize, inputSize);
    auto chunk = storage::TableRangeView::create(inputTable, start, end);
    emitChunk(chunk);
  }

  LOG4CXX_DEBUG(_pipelineLogger, "Done chunking.");
}

std::shared_ptr<PlanOperation> PipelineStream::parse(const Json::Value& data) {
  auto ps = std::make_shared<PipelineStream>();
  ps->_chunkSize = data["chunkSize"].asUInt();
  return ps;
}
}
}
