// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_TASKLISTBUILDER_H_
#define SRC_LIB_ACCESS_SQL_TASKLISTBUILDER_H_

#include "access/sql/typedef_helper.h"

namespace hyrise {
namespace access {
namespace sql {


class TaskListBuilder {
 public:
  TaskListBuilder(std::string id_prefix);
  ~TaskListBuilder();
  
  
  const task_list_t& getTaskList() const;

  bool hasCommit() const;
  void setHasCommit(bool b);

  void addPlanOp(plan_op_t plan_op, std::string name);
  void addPlanOp(plan_op_t plan_op, std::string name, task_t dependency);
  void addPlanOp(plan_op_t plan_op, std::string name, TransformationResult& meta);

  void addCommit(TransformationResult& meta);
  void addNoOp(TransformationResult& meta);
  void addValidatePositions(TransformationResult& meta);

 private:
  /**
   * Returns the next ID for an operator
   * @return  next ID for an operator
   */
  inline std::string nextTaskId();


  task_list_t _task_list;
  std::string _id_prefix;
  int _last_task_id;
  bool _has_commit;
};










} // namespace sql
} // namespace access
} // namespace hyrise
#endif