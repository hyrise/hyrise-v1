// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_TYPEDEF_H_
#define SRC_LIB_ACCESS_SQL_TYPEDEF_H_

#include <access/sql/parser/Expr.h>
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
  enum AbstractDataType {
    kUnknown,
    kInteger,
    kFloat,
    kString
  };

  // Information about the table
  // TODO: Create Field type
  // TODO: store aliases
  std::vector<std::string> fields;
  std::vector<std::string> keys;
  std::vector<AbstractDataType> data_types;
  std::string name;

  inline void addField(const std::string& field, const std::string& key, AbstractDataType type) {
    fields.push_back(field);
    keys.push_back(key);
    data_types.push_back(type);
  }
  inline void addFieldFromInfo(const TableInfo& info, const int id) {
    addField(info.fields[id], info.keys[id], info.data_types[id]);
  }

  inline int getFieldID(const hsql::Expr* expr) const {
    if (expr->table == nullptr) return getFieldID(expr->name);
    else return getFieldID(expr->name, expr->table);
  }
  
  inline int getFieldID(const std::string& field) const {
    return getFieldID(field, "");

  }
  inline int getFieldID(const std::string& field, const std::string& key) const {
    int count = std::count(fields.begin(), fields.end(), field);
    if (count == 0) return -1;
    if (count == 1) return std::find(fields.begin(), fields.end(), field) - fields.begin();

    // Try to find a match over the column keys (table names)
    for (auto it = std::find(fields.begin(), fields.end(), field);
         it != fields.end();
         it = std::find(++it, fields.end(), field)) {

      int id = it - fields.begin();
      if (key.compare(keys[id]) == 0) return id;
    }

    // If no match was found return the first column that matched
    return std::find(fields.begin(), fields.end(), field) - fields.begin();
  }

  inline bool containsField(const std::string& field) const { return std::find(fields.begin(), fields.end(), field) != fields.end(); }
  inline bool hasName(const std::string& name2) const { return name2.compare(name) == 0; }
  inline size_t numColumns() const { return fields.size(); }

  void useColumnInfoFrom(const TableInfo& other) {
    fields      = other.fields;
    keys        = other.keys;
    data_types  = other.data_types;
  }
};

struct TransformationResult : TableInfo {
  task_t first_task;
  task_t last_task;

  void append(const TransformationResult& other) {
    fields     = other.fields;
    keys       = other.keys;
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