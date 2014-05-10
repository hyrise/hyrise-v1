// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PipeliningTableScan.h"

#include "access/expressions/ExampleExpression.h"
#include "access/expressions/pred_SimpleExpression.h"
#include "access/expressions/ExpressionRegistration.h"
#include "storage/PointerCalculator.h"
#include "storage/TableRangeView.h"
#include "helper/types.h"
#include "helper/make_unique.h"

#include "log4cxx/logger.h"

#include "access/UnionAll.h"
#include "access/system/ResponseTask.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<PipeliningTableScan>("PipeliningTableScan");
log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access"));
}

PipeliningTableScan::PipeliningTableScan(std::unique_ptr<AbstractExpression> expr) : _expr(std::move(expr)) {}

void PipeliningTableScan::setupPlanOperation() {
  const auto& table = getInputTable();

  // TODO can this also be moved in the PipelineObserver interface?
  if (!table) {
    return;
  }

  auto tablerange = std::dynamic_pointer_cast<const storage::TableRangeView>(table);
  if (tablerange) {
    _table = tablerange->getActualTable();
    _expr->walk({tablerange->getActualTable()});
  } else {
    _table = table;
    _expr->walk({table});
  }
}

void PipeliningTableScan::executePlanOperation() {
  // TODO can we take this and move it into the PipelineObserver interface?
  if (!getInputTable()) {
    return;
  }
  size_t start, stop;
  const auto& tablerange = std::dynamic_pointer_cast<const storage::TableRangeView>(getInputTable());
  if (tablerange) {
    start = tablerange->getStart();
    stop = start + tablerange->size();
  } else {
    start = 0;
    stop = getInputTable()->size();
  }

  // When the input is 0, dont bother trying to generate results
  pos_list_t* positions = new pos_list_t();
  if (stop - start > 0)
    // scan in 100K chunks
    for (size_t chunk = 0; chunk < ((stop - start) / 100000 + 1); ++chunk) {
      size_t partial_start = start + chunk * 100000;
      size_t partial_stop = std::min(partial_start + 100000, stop);
      _expr->match(positions, partial_start, partial_stop);
      if (positions->size() >= _chunkSize) {
        createAndEmitChunk(positions);
        positions = new pos_list_t();
      }
    }
  else {
    createAndEmitChunk(positions);
  }

  // emit final chunk
  if (positions->size()) {
    createAndEmitChunk(positions);
  }
}

void PipeliningTableScan::createAndEmitChunk(pos_list_t* positions) {
  emitChunk(storage::PointerCalculator::create(_table, positions));
}

std::shared_ptr<AbstractPipelineObserver> PipeliningTableScan::clone() {
  auto clone = std::make_shared<PipeliningTableScan>(_expr->clone());
  clone->_chunkSize = _chunkSize;
  return clone;
}

std::shared_ptr<PlanOperation> PipeliningTableScan::parse(const Json::Value& data) {
  auto instance = std::make_shared<PipeliningTableScan>(Expressions::parse(data["expression"].asString(), data));
  instance->_chunkSize = data["chunkSize"].asUInt();
  return instance;
}
}
}
