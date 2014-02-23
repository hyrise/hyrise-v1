// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AgingCheck.h"

#include <iostream>

#include <storage/storage_types.h>
#include <storage/storage_types_helper.h>
#include <storage/BaseDictionary.h>
#include <io/StorageManager.h>
#include <access/aging/QueryManager.h>
#include <access/system/QueryParser.h>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<AgingCheck>("AgingCheck");
} // namespace


AgingCheck::AgingCheck(const std::string& query) :
  PlanOperation(),
  _queryName(query) {}

AgingCheck::~AgingCheck() {
  for (const auto& param : _paramList) {
    switch (param.type) {
      case IntegerType: delete (hyrise_int_t*) param.data; break;
      case FloatType: delete (hyrise_float_t*) param.data; break;
      case StringType: delete (hyrise_string_t*) param.data; break;
      default: throw std::runtime_error("unsupported field type");
    }
  }
}

void AgingCheck::executePlanOperation() {
  const auto& qm = QueryManager::instance();
  const auto& sm = *io::StorageManager::getInstance();

  if (!qm.exists(_queryName)) {
    output = input;
    std::cout << "query not registered => COLD" << std::endl;
    return;
  }
  const query_t query = qm.getId(_queryName);

  const auto& selection = qm.selectExpressionOf(query);
  const auto& tableNames = selection->accessedTables();
  std::cout << ">>>>";
  for (const auto t : tableNames)
    std::cout << t << ", ";
  std::cout << std::endl;
  std::vector<storage::c_atable_ptr_t> tables;
  for (const auto& tableName : tableNames)
    tables.push_back(sm.get<storage::AbstractTable>(tableName));

  const auto& inputTables = input.all();
  for (const auto& inputTable : inputTables) {
    const auto& casted = std::dynamic_pointer_cast<const storage::AbstractTable>(inputTable);
    if (casted == nullptr)
      throw std::runtime_error("at least one input resource is not a table");

    // this needs not to be optimized as usuall queries will only one or two tables
    size_t i;
    for (i = 0; i < tables.size(); ++i) {
      if (tables.at(i) == casted)
        break;
    }
    if (i == tables.size()) {
      output.add(casted);
      std::cout << "\t found irrelevant table" << std::endl;
      continue; // table is irrelevant for aging - but should not occure either
    }

    const auto& tableName = tableNames.at(i);
    std::cout << "\t found relevant table: " << tableName << std::endl;

    const auto& fieldNames = selection->accessedFields(tableName);

    std::vector<storage::aging_index_ptr_t> agingIndices;
    std::cout << "\t>>>>";
    for (const auto fieldName : fieldNames) {
      std::cout << fieldName << ", ";
      if (sm.hasAgingIndex(tableName, fieldName))
        agingIndices.push_back(sm.getAgingIndexFor(tableName, fieldName));
    }
    std::cout << std::endl;

    if (agingIndices.size() == 0) {
      std::cout << "found no aging information at all => COLD" << std::endl;
      continue;
    }
    
    std::cout << "Woohoo, found at least one AgingIndex" << std::endl;

    const auto& fields = selection->accessedFields(tableName);
  }

  

  output = input; //TODO rethink maybe


  while (_paramList.size() > 0 ) {
    const auto ret = handleOneTable(_paramList);
    std::cout << ">>>>>>>>>" << ret.first << ": " << (ret.second ? "HOT" : "COLD") << std::endl;
  }

  /*for (const auto& param : _paramList) {
    sm.assureExists(param.table);

    const auto& table = sm.get<storage::AbstractTable>(tableName);

    bool hot = true;
    for (const auto& field : tableParams.second) {
      const field_t col = table->numberOfColumn(field.name);

      if (!sm.hasAgingIndex(tableName, field.name)) {
        std::cout << "no aging index => COLD"; //TODO checkme
        continue;
      }

      if (table->typeOfColumn(col) != field.type)
        throw std::runtime_error("type of " + field.name + " is of type " +
                                 data_type_to_string(field.type));

      value_id_t vid;

      switch (field.type) {
        case IntegerType:
          vid = table->getValueIdForValue<hyrise_int_t>(col, *(hyrise_int_t*)field.data).valueId;
          std::cout << "<<" << *(hyrise_int_t*)field.data << ">>" << std::endl;
          break;
        case FloatType:
          vid = table->getValueIdForValue<hyrise_float_t>(col, *(hyrise_float_t*)field.data).valueId;
          std::cout << "<<" << *(hyrise_float_t*)field.data << ">>" << std::endl;
          break;
        case StringType:
          vid = table->getValueIdForValue<hyrise_string_t>(col, *(hyrise_string_t*)field.data).valueId;
          std::cout << "<<" << *(hyrise_string_t*)field.data << ">>" << std::endl;
          break;
        default: throw std::runtime_error("unsupported field type");
      }

      if (!sm.getAgingIndexFor(tableName, field.name)->isHot(qm.getId(_queryName), vid)) {
        hot = false;
        std::cout << tableName << ":" << field.name << " COLD" << std::endl;
        break;
      }
      std::cout << tableName << ":" << field.name << " HOT" << std::endl;
    }

    std::cout << ">>>>>>>>>" << tableName << ": " << (hot ? "HOT" : "COLD") << std::endl;
  }*/
}

void AgingCheck::parameter(const std::vector<param_data_t>& parameter) {
  _paramList = parameter;
}

std::pair<std::string, bool> AgingCheck::handleOneTable(std::vector<param_data_t>& paramList) {
  if (paramList.size() == 0)
    throw std::runtime_error("this should not happen");
  const std::string table = paramList[0].table;

  //TODO
  //const auto& sm = *io::StorageManager::getInstance();
  //const auto& qm = QueryManager::instance();

  auto it = paramList.begin();
  while(it != paramList.end()) {
    if ((*it).table == table) {
      //TODO

      std::cout << "AgingCheck for " << table << "." << (*it).field << std::endl;
      paramList.erase(it);
    }
    else {
      ++it;
    }
  }

  return std::make_pair(table, true);
}

std::shared_ptr<PlanOperation> AgingCheck::parse(const Json::Value &data) {
  if (!data.isMember("query"))
    throw std::runtime_error("A query must be specified for the AgingCheck");
  const auto& queryName = data["query"].asString();

  std::shared_ptr<AgingCheck> ac = std::make_shared<AgingCheck>(queryName);

  if (data.isMember("parameters")) {
    const auto& params = data["parameters"];
    for (unsigned i = 0; i < params.size(); ++i) {
      const auto& param = params[i];
      if (!param.isMember("table"))
        throw std::runtime_error("A table needs to be specified for a parameter");
      const auto& tableName = param["table"].asString();

      if (!param.isMember("field"))
        throw std::runtime_error("A field needs to be specified for a parameter");
      const auto& fieldName = param["field"].asString();

      if (!param.isMember("value"))
        throw std::runtime_error("A value needs to be specified for a parameter");
      const auto& value = param["value"];
      if (value.isInt())
        ac->_paramList.push_back({tableName, fieldName, IntegerType, new hyrise_int_t(value.asInt())});
      else if (value.isDouble())
        ac->_paramList.push_back({tableName, fieldName, FloatType, new hyrise_float_t(value.asDouble())});
      else if (value.isString())
        ac->_paramList.push_back({tableName, fieldName, StringType, new hyrise_string_t(value.asString())});
      else
        throw std::runtime_error("cannot recognize data type");
    }
  }

  return ac;
}

} } // namespace hyrise::access

