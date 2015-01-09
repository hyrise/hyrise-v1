// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "TaskListBuilder.h"

#include "access/tx/ValidatePositions.h"
#include "access/tx/Commit.h"
#include "access/NoOp.h"

namespace hyrise {
namespace access {
namespace sql {


TaskListBuilder::TaskListBuilder(std::string id_prefix) :
  _id_prefix(id_prefix),
  _last_task_id(0),
  _has_commit(false) { /* initialize */ }


TaskListBuilder::~TaskListBuilder() { /* destruct */ }


std::string TaskListBuilder::nextTaskId() {
  return _id_prefix + std::to_string(++_last_task_id);
}


bool TaskListBuilder::hasCommit() const {
  return _has_commit;
}


void TaskListBuilder::setHasCommit(bool b) {
  _has_commit = b;
}


const task_list_t& TaskListBuilder::getTaskList() const {
  return _task_list;
}


void TaskListBuilder::addPlanOp(plan_op_t op, std::string name) {
  op->setPlanOperationName(name);
  op->setOperatorId(nextTaskId() + " " + name);
  _task_list.push_back(op);
}


void TaskListBuilder::addPlanOp(plan_op_t op, std::string name, task_t dependency) {
  if (dependency != nullptr) op->addDependency(dependency);
  addPlanOp(op, name);
}


void TaskListBuilder::addPlanOp(plan_op_t op, std::string name, TransformationResult& meta) {
  task_t dependency = meta.last_task;
  meta.addTask(op);
  addPlanOp(op, name, dependency);
}


void TaskListBuilder::addCommit(TransformationResult& meta) { 
  auto op = std::make_shared<Commit>();
  setHasCommit(true);
  addPlanOp(op, "Commit", meta);
}


void TaskListBuilder::addValidatePositions(TransformationResult& meta) { 
  auto op = std::make_shared<ValidatePositions>();
  addPlanOp(op, "ValidatePositions", meta);
}


void TaskListBuilder::addNoOp(TransformationResult& meta) { 
  auto op = std::make_shared<NoOp>();
  addPlanOp(op, "NoOp", meta);
}






} // namespace sql
} // namespace access
} // namespace hyrise