// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/InsertScan.h"

#include "json_converters.h"

#include "helper/vector_helpers.h"
#include "helper/checked_cast.h"

#include "io/TransactionManager.h"
#include "storage/Store.h"
#include "storage/meta_storage.h"


namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<InsertScan>("InsertScan");
}



InsertScan::~InsertScan() {
}

storage::atable_ptr_t InsertScan::buildFromJson() {

  auto result = input.getTable()->copy_structure_modifiable();
  result->resize(_raw_data.size());

  set_json_value_functor fun(result);
  storage::type_switch<hyrise_basic_types> ts;

  auto col_count = input.getTable()->columnCount();
  for (size_t r=0, row_count=_raw_data.size(); r < row_count; ++r ) {
    for(size_t c=0; c < col_count; ++c) {
      fun.set(c,r,_raw_data[r][c]);
      ts(result->typeOfColumn(c), fun);
    }    
  }

  return result;
  
}

void InsertScan::executePlanOperation() {
  const auto& c_store = checked_pointer_cast<const Store>(input.getTable(0));
  
  // Cast the constness away
  auto store = std::const_pointer_cast<Store>(c_store);

  if (!_data)
    _data = buildFromJson();

  // Delta Table Size
  size_t max = store->getDeltaTable()->size();
  const auto& beforSize = store->size();

  auto writeArea = store->resizeDelta(max + _data->size());
  const auto& hidden = false;

  // Get the modifications record
  auto& mods = tx::TransactionManager::getInstance()[_txContext.tid];
  for(size_t i=0, upper = _data->size(); i < upper; ++i) {
    store->copyRowToDelta(_data, i, writeArea.first+i, _txContext.tid, hidden);
    mods.insertPos(store, beforSize+i);
  }

  addResult(input.getTable(0));
}

void InsertScan::setInputData(const storage::atable_ptr_t &c) {
  _data = c;
}

std::shared_ptr<_PlanOperation> InsertScan::parse(Json::Value &data) {
  auto result = std::make_shared<InsertScan>();

  if (data.isMember("data")) {
    result->_raw_data = functional::collect(data["data"], [](const Json::Value& v){
      return functional::collect(v, [](const Json::Value& c){ return Json::Value(c); });
    });
  }
  return result;
}

}
}
