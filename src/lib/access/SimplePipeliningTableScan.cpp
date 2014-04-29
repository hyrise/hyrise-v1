// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SimplePipeliningTableScan.h"

#include "access/expressions/pred_buildExpression.h"

#include "access/PipelineObserver.h"

#include "storage/Store.h"
#include "storage/PointerCalculator.h"
#include "storage/AbstractTable.h"

#include "helper/checked_cast.h"
#include "taskscheduler/SharedScheduler.h"
#include "access/system/ResponseTask.h"
#include "access/system/OperationData-Impl.h"

#include "log4cxx/logger.h"

#include <boost/uuid/uuid.hpp>  // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators
#include <boost/uuid/uuid_io.hpp>  // streaming operators etc.

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<SimplePipeliningTableScan>("SimplePipeliningTableScan");
log4cxx::LoggerPtr _pipelineLogger(log4cxx::Logger::getLogger("pipelining.SimplePipeliningTableScan"));
}

SimplePipeliningTableScan::SimplePipeliningTableScan() : _comparator(nullptr) {}

SimplePipeliningTableScan::~SimplePipeliningTableScan() {
  if (_comparator)
    delete _comparator;
}

void SimplePipeliningTableScan::setupPlanOperation() {
  // check if any input tables are available.
  if (input.sizeOf<storage::AbstractTable>() == 0) {
    _tbl = nullptr;
    return;
  }

  setPredicate(buildExpression(_predicates));
  _comparator->walk(input.getTables());
  _tbl = input.getTable(0);
}

void SimplePipeliningTableScan::executePlanOperation() {
  // if there is no input data, just return.
  if (!_tbl) {
    LOG4CXX_DEBUG(_pipelineLogger, "executePlanOperation: no input. Just return.");
    return;
  }

  LOG4CXX_DEBUG(_pipelineLogger, "executePlanOperation: input available. Normal operation.");

  size_t row = _ofDelta ? checked_pointer_cast<const storage::Store>(_tbl)->deltaOffset() : 0;
  for (size_t input_size = _tbl->size(); row < input_size; ++row) {
    if ((*_comparator)(row)) {
      _pos_list->push_back(row);
      if (_pos_list->size() > _chunkSize) {
        emitChunk();
      }
    }
  }
  if (_pos_list->size() > 0) {
    LOG4CXX_DEBUG(_pipelineLogger, "Emitting final chunk.");
    emitChunk();
  } else {
    LOG4CXX_DEBUG(_pipelineLogger, "No final chunk necessary.");
  }
}

void SimplePipeliningTableScan::emitChunk() {
  LOG4CXX_DEBUG(_pipelineLogger, "Emitting new chunk.");
  auto chunk = storage::PointerCalculator::create(_tbl, _pos_list);
  PipelineEmitter::emitChunk(chunk);
  _pos_list = new pos_list_t();
}

std::shared_ptr<PlanOperation> SimplePipeliningTableScan::parse(const Json::Value& data) {
  std::shared_ptr<SimplePipeliningTableScan> pop = std::make_shared<SimplePipeliningTableScan>();

  if (!data.isMember("predicates")) {
    throw std::runtime_error("There is no reason for a Selection without predicates");
  }
  pop->_predicates = data["predicates"];

  if (data.isMember("ofDelta")) {
    pop->_ofDelta = data["ofDelta"].asBool();
  }

  if (!data.isMember("chunkSize")) {
    throw std::runtime_error("You have to provide a chunk size for pipelined operators");
  }
  pop->_chunkSize = data["chunkSize"].asUInt();

  return pop;
}

std::shared_ptr<PlanOperation> SimplePipeliningTableScan::copy() {
  auto plop = std::make_shared<SimplePipeliningTableScan>();
  plop->_predicates = _predicates;
  plop->_ofDelta = _ofDelta;
  plop->_chunkSize = _chunkSize;
  return plop;
}

void SimplePipeliningTableScan::setPredicate(SimpleExpression* c) { _comparator = c; }
}
}
