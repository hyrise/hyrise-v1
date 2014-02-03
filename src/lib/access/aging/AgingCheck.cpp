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
  for (const auto& table : _fields) {
    const auto tableParams = table.second;
    for (const auto& field : tableParams) {
      switch (field.type) {
        case IntegerType: delete (hyrise_int_t*) field.data; break;
        case FloatType: delete (hyrise_float_t*) field.data; break;
        case StringType: delete (hyrise_string_t*) field.data; break;
        default: throw std::runtime_error("unsupported field type");
      }
    }
  }
}

void AgingCheck::executePlanOperation() {
  const auto& sm = *io::StorageManager::getInstance();
  const auto& qm = QueryManager::instance();

  qm.assureExists(_queryName);
  for (const auto& tableParams : _fields) {
    const auto& tableName = tableParams.first;
    sm.assureExists(tableName);

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
  }

  output = input; // don't stand in the way of calculation, pass everything on
}

std::shared_ptr<PlanOperation> AgingCheck::parse(const Json::Value &data) {
  std::shared_ptr<AgingCheck> ac = std::make_shared<AgingCheck>();

  if (!data.isMember("query"))
    throw std::runtime_error("A query must be specified for the AgingCheck");
  ac->_queryName = data["query"].asString();

  if (data.isMember("parameters")) {
     const auto& params = data["parameters"];
     for (unsigned i = 0; i < params.size(); ++i) {
       const auto& table = params[i];
       if (!table.isMember("table"))
         throw std::runtime_error("A table needs to be specified for a parameter list");
       const auto& tableName = table["table"].asString();

       field_data_list_t fieldVector;
       if (table.isMember("parameters")) {
         const auto& fields = table["parameters"];
         for (unsigned j = 0; j < fields.size(); ++j) {
           const auto& fieldData = fields[j];
           if (fieldData.size() != 2)
             throw std::runtime_error("there have to be exactly two members, one"
                                      " for the field name and one for the value");
           const auto& fieldName = fieldData[0].asString();
           const auto& value = fieldData[1];

           if (value.isInt())
             fieldVector.push_back({fieldName, IntegerType, new hyrise_int_t(value.asInt())});
           else if (value.isDouble())
             fieldVector.push_back({fieldName, FloatType, new hyrise_float_t(value.asDouble())});
           else if (value.isString())
             fieldVector.push_back({fieldName, StringType, new hyrise_string_t(value.asString())});
           else
             throw std::runtime_error("cannot recognize data type");
         }
       }

       ac->_fields.insert(std::make_pair(tableName, fieldVector));
     }
  }

  return ac;
}

} } // namespace hyrise::access

