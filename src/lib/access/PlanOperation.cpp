// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "PlanOperation.h"

#include <algorithm>
#include <thread>

#include <boost/lexical_cast.hpp>

#include "helper/epoch.h"
#include "helper/PapiTracer.h"
#include "io/StorageManager.h"
#include "storage/AbstractResource.h"
#include "storage/AbstractHashTable.h"
#include "storage/AbstractTable.h"
#include "storage/TableRangeViewFactory.h"
#include "storage/TableRangeView.h"

namespace { log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("access.plan._PlanOperation")); }

_PlanOperation::_PlanOperation() :
      _limit(0),
      _part(0),
      _count(0),
      producesPositions(true),
      _planId(),
      _operatorId() {}

_PlanOperation::~_PlanOperation() {
}

void _PlanOperation::addResult(hyrise::storage::c_atable_ptr_t result) {
  output.add(result);
}

void _PlanOperation::addResultHash(hyrise::storage::c_ahashtable_ptr_t result) {
  output.addHash(result);
}

void _PlanOperation::addInput(std::vector<hyrise::storage::c_atable_ptr_t> *input_list) {
  for (const auto& t: *input_list)
      addInput(t);
}

void _PlanOperation::addInput(std::vector<hyrise::storage::c_ahashtable_ptr_t> *input_list) {
  for (const auto& t: *input_list)
      addInputHash(t);
}


const hyrise::storage::c_atable_ptr_t _PlanOperation::getInputTable(size_t index) const {
  return input.getTable(index);
}

static const hyrise::storage::atable_ptr_t empty_result;

const hyrise::storage::c_atable_ptr_t _PlanOperation::getResultTable(size_t index) const {
  if (output.numberOfTables())
    return output.getTable(index);
  else
     return empty_result;
}

hyrise::storage::c_ahashtable_ptr_t _PlanOperation::getInputHashTable(size_t index) const {
  return input.getHashTable(index);
}

hyrise::storage::c_ahashtable_ptr_t _PlanOperation::getResultHashTable(size_t index) const {
  return output.getHashTable(index);
}

bool _PlanOperation::allDependenciesSuccessful() {
  for (size_t i = 0; i < _dependencies.size(); ++i) {
    if (std::dynamic_pointer_cast<OutputTask>(_dependencies[i])->getState() == OpFail) return false;
  }
  return true;
}

std::string _PlanOperation::getDependencyErrorMessages() {
  std::string result;

  std::shared_ptr<OutputTask> task;
  for (size_t i = 0; i < _dependencies.size(); ++i) {
    task = std::dynamic_pointer_cast<OutputTask>(_dependencies[i]);
    if (task->getState() == OpFail) result += task->getErrorMessage() + "\n";
  }

  return result;
}

void _PlanOperation::setFields(field_list_t *f) {
  _indexed_field_definition = field_list_t(*f);
}

void _PlanOperation::addField(field_t field) {
  _indexed_field_definition.push_back(field);
}


void _PlanOperation::addNamedField(const field_name_t& field) {
  _named_field_definition.push_back(field);
}

void _PlanOperation::addField(const Json::Value &field) {
  if (field.isNumeric()) {
    addField(field.asUInt());
  } else if (field.isString()) {
    addNamedField(field.asString());
  } else throw std::runtime_error("Can't parse field name, neither numeric nor std::string");
}

/* This method only returns the column number in each table, assuming the operation knows how to handle positions */
unsigned int _PlanOperation::findColumn(const std::string &col) {
  for (const auto& table: input.getTables()) {
    try {
      return table->numberOfColumn(col);
    } catch (MissingColumnException e) {}
  }
  throw MissingColumnException(col);
}

size_t widthOfInputs(const std::vector<hyrise::storage::c_atable_ptr_t > &inputs) {
  size_t result = 0;
  for (const auto& table: inputs) {
    result += table->columnCount();
  }
  return result;
}

void _PlanOperation::computeDeferredIndexes() {
  _field_definition = _indexed_field_definition;
  if (!_named_field_definition.empty()) {

    if ((_named_field_definition.size() == 1) && (_named_field_definition[0] == "*")) {
      for (size_t field_index = 0; field_index < widthOfInputs(input.getTables()); ++field_index) {
        _field_definition.push_back(field_index);
      }
    } else {
      for (size_t i = 0; i < _named_field_definition.size(); ++i) {
        _field_definition.push_back(findColumn(_named_field_definition[i]));
      }
    }
  }
  assert(_field_definition.size() >= (_indexed_field_definition.size() + _named_field_definition.size()));
}

void _PlanOperation::setupPlanOperation() {
  computeDeferredIndexes();
}

void _PlanOperation::distribute(
    const u_int64_t numberOfElements,
    u_int64_t &first,
    u_int64_t &last) const {

  const u_int64_t
      elementsPerPart     = numberOfElements / _count;

  first = elementsPerPart * _part;
  last = _part + 1 == _count ? numberOfElements : elementsPerPart * (_part + 1);
}

void _PlanOperation::refreshInput() {
  size_t numberOfDependencies = _dependencies.size();
  for (size_t i = 0; i < numberOfDependencies; ++i) {
    const auto& dependency = std::dynamic_pointer_cast<_PlanOperation>(_dependencies[i]);
    input.mergeWith(dependency->output);
  }

  splitInput();
}

void _PlanOperation::splitInput() {
  const auto& tables = input.getTables();
  if (_count > 0 && !tables.empty()) {
    u_int64_t first, last;
    distribute(tables[0]->size(), first, last);
    input.setTable( TableRangeViewFactory::createView(std::const_pointer_cast<AbstractTable>(tables[0]), first, last), 0);
  }
}


void _PlanOperation::operator()() noexcept {
  if (allDependenciesSuccessful()) {
    try {
      LOG4CXX_DEBUG(logger, "Virtual operator called " << vname() << "(" << _operatorId << ")");
      execute();
      return;
    } catch (const std::exception &ex) {
      setErrorMessage(ex.what());
    } catch (...) {
      setErrorMessage("Unknown error");
    }
  } else { // dependencies were not successful
    setErrorMessage(getDependencyErrorMessages());
  }
  LOG4CXX_ERROR(logger, this << " " << planOperationName() << " failed: " << getErrorMessage());
  setState(OpFail);
}

const _PlanOperation *_PlanOperation::execute() {
  epoch_t startTime = get_epoch_nanoseconds();

  refreshInput();

  setupPlanOperation();

  PapiTracer pt;
  pt.addEvent("PAPI_TOT_CYC");
  pt.addEvent(getEvent());

  pt.start();
  executePlanOperation();
  pt.stop();

  teardownPlanOperation();

  epoch_t endTime = get_epoch_nanoseconds();
  std::string threadId = boost::lexical_cast<std::string>(std::this_thread::get_id());

  if (_performance_attr != nullptr)
    *_performance_attr = (performance_attributes_t) {
      pt.value("PAPI_TOT_CYC"), pt.value(getEvent()), getEvent() , planOperationName(), _operatorId, startTime, endTime, threadId
    };

  setState(OpSuccess);
  return this;
}

void _PlanOperation::setLimit(uint64_t l) {
  _limit = l;
}

void _PlanOperation::setProducesPositions(bool p) {
  producesPositions = p;
}

void _PlanOperation::setTransactionId(hyrise::tx::transaction_id_t tid) {
  _transaction_id = tid;
}

void _PlanOperation::addInput(hyrise::storage::c_atable_ptr_t t) {
  input.add(t);
}
void _PlanOperation::addInputHash(hyrise::storage::c_ahashtable_ptr_t t) {
  input.addHash(t);
}

void _PlanOperation::setPart(size_t part) {
    _part = part;
}
void _PlanOperation::setCount(size_t count) {
  _count = count;
}

void _PlanOperation::setPlanId(std::string i) {
  _planId = i;
}
void _PlanOperation::setOperatorId(std::string i) {
  _operatorId = i;
}

const std::string& _PlanOperation::planOperationName() const {
  return _planOperationName;
}
void _PlanOperation::setPlanOperationName(const std::string& name) {
  _planOperationName = name;
}

const std::string _PlanOperation::vname() {
  return planOperationName();
}

void _PlanOperation::setResponseTask(const std::shared_ptr<hyrise::access::ResponseTask>& responseTask) {
  _responseTask = responseTask;
}

std::shared_ptr<hyrise::access::ResponseTask> _PlanOperation::getResponseTask() const {
  return _responseTask.lock();
}
