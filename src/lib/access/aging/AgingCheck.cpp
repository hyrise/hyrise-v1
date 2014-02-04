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
  output = input; //TODO rethink maybe

  const auto& qm = QueryManager::instance();
  if (!qm.exists(_queryName)) {
    std::cout << "query not registered => COLD" << std::endl;
    return;
  }

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

std::pair<std::string, bool> AgingCheck::handleOneTable(std::vector<param_data_t>& paramList) {
  if (paramList.size() == 0)
    throw std::runtime_error("this should not happen");
  const std::string table = paramList[0].table;

  const auto& sm = *io::StorageManager::getInstance();
  const auto& qm = QueryManager::instance();

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
  std::shared_ptr<AgingCheck> ac = std::make_shared<AgingCheck>();

  if (!data.isMember("query"))
    throw std::runtime_error("A query must be specified for the AgingCheck");
  ac->_queryName = data["query"].asString();

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

