// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/InsertOnlyOperators.h"

#include "access/insertonly.h"
#include "access/system/QueryParser.h"
#include "io/StorageManager.h"
#include "storage/SimpleStore.h"
#include "storage/PointerCalculator.h"

namespace hyrise {
namespace insertonly {

using access::PlanOperation;

namespace {
using access::QueryParser;
auto register_load = QueryParser::registerPlanOperation<LoadOp>("InsertOnlyLoad");
auto register_insert = QueryParser::registerPlanOperation<InsertOp>("InsertOnlyInsert");
auto register_update = QueryParser::registerPlanOperation<UpdateOp>("InsertOnlyUpdate");
auto register_delete = QueryParser::registerPlanOperation<DeleteOp>("InsertOnlyDelete");
auto register_valid = QueryParser::registerPlanOperation<ValidPositionsRawOp>("ValidPositionsRaw");
auto register_valid_main = QueryParser::registerPlanOperation<ValidPositionsMainOp>("ValidPositionsMain");
auto register_delta_extract = QueryParser::registerPlanOperation<ExtractDelta>("ExtractDelta");
}

LoadOp::LoadOp(const std::string &filename) : _filename(filename) {
}

const std::string LoadOp::vname() {
  return "InsertOnlyLoad";
}

std::shared_ptr<PlanOperation> LoadOp::parse(Json::Value &data) {
  return std::make_shared<LoadOp>(data["filename"].asString());
}

void LoadOp::executePlanOperation() {
  addResult(construct(StorageManager::getInstance()->makePath(_filename), hyrise::tx::TXContext(0,0)));
}

const std::string InsertOp::vname() {
  return "InsertOnlyInsert";
}

std::shared_ptr<PlanOperation> InsertOp::parse(Json::Value &data) {
  return std::make_shared<InsertOp>();
}

void InsertOp::executePlanOperation() {
  auto table = std::const_pointer_cast<AbstractTable>(getInputTable(0));
  auto store = assureInsertOnly(table);
  auto rows = getInputTable(1);
  insertRows(store, rows, _txContext);
  addResult(table);
}

const std::string UpdateOp::vname() {
  return "InsertOnlyUpdate";
}

std::shared_ptr<PlanOperation> UpdateOp::parse(Json::Value &data) {
  return std::make_shared<UpdateOp>();
}

void UpdateOp::executePlanOperation() {
  auto store = assureInsertOnly(std::const_pointer_cast<AbstractTable>(getInputTable(0)));
  auto rows = getInputTable(1);
  auto positions_main = std::dynamic_pointer_cast<const PointerCalculator>(getInputTable(2))->getPositions();
  auto positions_delta = std::dynamic_pointer_cast<const PointerCalculator>(getInputTable(3))->getPositions();

  updateRows(store, rows, *positions_main, *positions_delta, _txContext);
  addResult(store);
}

const std::string DeleteOp::vname() {
  return "InsertOnlyDelete";
}

std::shared_ptr<PlanOperation> DeleteOp::parse(Json::Value &data) {
  return std::make_shared<DeleteOp>();
}

void DeleteOp::executePlanOperation() {
  auto store = assureInsertOnly(std::const_pointer_cast<AbstractTable>(getInputTable(0)));
  auto positions_main = std::dynamic_pointer_cast<const PointerCalculator>(getInputTable(1))->getPositions();
  auto positions_delta = std::dynamic_pointer_cast<const PointerCalculator>(getInputTable(2))->getPositions();

  deleteRows(store, *positions_main, *positions_delta, _txContext);
  addResult(store);
}

const std::string ValidPositionsRawOp::vname() {
  return "ValidPositionsRaw";
}

std::shared_ptr<PlanOperation> ValidPositionsRawOp::parse(Json::Value &data) {
  return std::make_shared<ValidPositionsRawOp>();
}

void ValidPositionsRawOp::executePlanOperation() {
  const auto& store = assureInsertOnly(std::const_pointer_cast<AbstractTable>(getInputTable(0)));
  addResult(validPositionsDelta(store, _txContext));
}

const std::string ValidPositionsMainOp::vname() {
  return "ValidPositionsMain";
}

std::shared_ptr<PlanOperation> ValidPositionsMainOp::parse(Json::Value &data) {
  return std::make_shared<ValidPositionsMainOp>();
}

void ValidPositionsMainOp::executePlanOperation() {
  const auto& store = assureInsertOnly(std::const_pointer_cast<AbstractTable>(getInputTable(0)));
  addResult(validPositionsMain(store, _txContext));
}

std::shared_ptr<PlanOperation> ExtractDelta::parse(Json::Value&) {
  return std::make_shared<ExtractDelta>();
}

void ExtractDelta::executePlanOperation() {
  const auto& store = std::dynamic_pointer_cast<const storage::SimpleStore>(getInputTable());
  if (store == nullptr) { throw std::runtime_error("Passed table is not a SimpleStore!"); }
  addResult(store->getDelta());
}

const std::string ExtractDelta::vname() {
  return "ExtractDelta";
};


}}
