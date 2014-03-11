// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AgingCheck.h"

//#include <iostream>

#include <storage/storage_types.h>
#include <storage/storage_types_helper.h>
#include <storage/BaseDictionary.h>
#include <storage/AgingStore.h>
#include <storage/StoreRangeView.h>

#include <io/StorageManager.h>
#include <access/aging/QueryManager.h>
#include <access/system/QueryParser.h>

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<AgingCheck>("AgingCheck");


struct create_data_functor {
  typedef void value_type;
  create_data_functor(void* data) : _data(data) {}

  template <typename T>
  void operator()() {
    data = new T(*_data);
  }

  void* data;

 private:
  void* _data;
};

struct delete_data_functor {
  typedef void value_type;
  delete_data_functor(void* data) : _data(data) {}

  template <typename T>
  void operator()() {
    delete (T*)_data;
  }

 private:
  void* _data;
};

} // namespace


AgingCheck::AgingCheck(const std::string& query) :
  PlanOperation(),
  _queryName(query) {}

AgingCheck::~AgingCheck() {
  for (const auto& param : _paramList) {
    delete_data_functor functor(param.data);
    storage::type_switch<hyrise_basic_types> ts;
    ts(param.type, functor);
  }
}


namespace {
struct get_vid_functor {
 public:
  typedef void value_type;

  get_vid_functor(storage::atable_ptr_t table, const param_data_t& data) :
    _table(table),
    _data(data) {}

  template <typename T>
  void operator()() {
    const auto& field = _table->numberOfColumn(_data.field);
    const auto& dict = checked_pointer_cast<storage::BaseDictionary<T>>(_table->dictionaryAt(field));

    vid = dict->getValueIdForValue(*(T*)_data.data);
  }

  storage::value_id_t vid;

 private:
  const storage::atable_ptr_t _table;
  const param_data_t& _data;
};

} // namespace

void AgingCheck::executePlanOperation() {
  const auto& qm = QueryManager::instance();
  const auto& sm = *io::StorageManager::getInstance();

  if (!qm.exists(_queryName)) {
    output = input;
    //std::cout << "query not registered => COLD" << std::endl;
    return;
  }
  const query_t query = qm.getId(_queryName);

  const auto& selection = qm.selectExpressionOf(query);
  const auto& tableNames = selection->accessedTables();

  std::vector<storage::c_atable_ptr_t> tables;
  for (const auto& tableName : tableNames)
    tables.push_back(sm.get<storage::AbstractTable>(tableName));

  const auto& inputResources = input.all();
  for (const auto& inputResource : inputResources) {
    const auto& nonConst = std::const_pointer_cast<storage::AbstractResource>(inputResource);
    const auto& agingStore = std::dynamic_pointer_cast<storage::AgingStore>(nonConst);
    if (agingStore == nullptr) {
      output.addResource(inputResource);
      //std::cout << "resource is not an AgingStore => COLD" << std::endl;
      continue;
    }

    // this needs not to be optimized as usuall queries will only one or two tables
    size_t i;
    for (i = 0; i < tables.size(); ++i) {
      if (tables.at(i) == agingStore)
        break;
    }
    if (i == tables.size()) {
      output.add(agingStore);
      //std::cout << "found irrelevant table" << std::endl;
      continue; // table is irrelevant for aging - but should not occure either
    }

    const auto& tableName = tableNames.at(i);
    const auto& fieldNames = selection->accessedFields(tableName);

    bool indexMissing = false;
    std::vector<storage::aging_index_ptr_t> agingIndices;
    for (const auto fieldName : fieldNames) {
      if (sm.hasAgingIndex(tableName, fieldName))
        agingIndices.push_back(sm.getAgingIndexFor(tableName, fieldName));
      else {
        indexMissing = true;
        break;
      }
    }

    if (indexMissing) {
      //std::cout << "Missing aging index for at least one field" << std::endl;
      output.add(agingStore);
      continue;
    }

    bool hot = true;
    bool skip = false;
    for (const auto& agingIndex : agingIndices) {
      const param_data_t* data = nullptr;
      const auto& fieldName = agingStore->nameOfColumn(agingIndex->field());
      for (const auto& param : _paramList) {
        if (param.table == tableName && param.field == fieldName) {
          data = &param;
          break;
        }
      }
      if (data == nullptr)
        throw std::runtime_error("no value specified for " + tableName + "." + fieldName);
      
      get_vid_functor functor(agingStore, *data);
      storage::type_switch<hyrise_basic_types> ts;
      ts(data->type, functor);

      if (!agingIndex->isVidRegistered(functor.vid)) {
        skip = true;
        break;
      }
      else if (!agingIndex->isHot(query, functor.vid)) {
        hot = false;
        break;
      }
    }

    if (skip) {
      //std::cout << "Value obviously not in table => SKIP" << std::endl;
      output.add(agingStore->getDeltaTable());
    }
    else if (hot) {
      //std::cout << "Hot-only scan => HOT" << std::endl;
      output.add(std::make_shared<storage::StoreRangeView>(agingStore, agingStore->hotSize()));
    }
    else {
      //std::cout << "Normal Scan => COLD" << std::endl;
      output.add(agingStore);
    }
  }
}

void AgingCheck::parameter(const std::vector<param_data_t>& parameter) {
  _paramList = parameter;
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

