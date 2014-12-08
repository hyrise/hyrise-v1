// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_TYPEDEF_H_
#define SRC_LIB_ACCESS_SQL_TYPEDEF_H_

#include <access/system/PlanOperation.h>
#include <taskscheduler/Task.h>
#include <storage/storage_types.h>

#include <vector>
#include <algorithm>

namespace hyrise {
namespace access {
namespace sql {



typedef std::shared_ptr<PlanOperation> plan_op_t;
typedef std::shared_ptr<taskscheduler::Task> task_t;
typedef std::vector<task_t> task_list_t;



struct TableInfo {
  // Information about the table
  std::vector<std::string> fields;
  std::vector<DataType> data_types;
  std::string name;

  inline void addField(std::string field) { fields.push_back(field); }
  inline void addField(std::string field, DataType type) { addField(field); data_types.push_back(type); }
  inline bool containsField(std::string field) const { return std::find(fields.begin(), fields.end(), field) != fields.end(); }
  inline bool hasName(std::string name2) const { return name2.compare(name) == 0; }
  inline size_t numColumns() const { return fields.size(); }
};

struct TransformationResult : TableInfo {
  task_t first_task;
  task_t last_task;

  void append(const TransformationResult& other) {
    fields     = other.fields;
    data_types = other.data_types;
    name       = other.name;
    last_task  = other.last_task;
  }

  void addTask(const task_t& task) {
  	if (first_task == nullptr) first_task = task;
  	last_task = task;
  }
};

// Zero initializes a Transformation Result struct without causing warnings
// TransformationResult t = ALLOC_TRANSFORMATIONRESULT();
#define ALLOC_TRANSFORMATIONRESULT() {}


} // namespace sql
} // namespace access
} // namespace hyrise
#endif