// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/InsertScan.h"
#include "json_converters.h"
#include "helper/vector_helpers.h"

#include "storage/Store.h"
#include "storage/meta_storage.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<InsertScan>("InsertScan");
}

struct set_value_functor {

  typedef void value_type;

  storage::atable_ptr_t tab;
  size_t col;
  size_t row;
  Json::Value val;


  set_value_functor(storage::atable_ptr_t t): tab(t) {
  }

  void set(size_t c, size_t r, Json::Value v) {
    col = c; row = r; val = v;
  }

  template<typename T>
  value_type operator()() {
    tab->setValue(col, row, json_converter::convert<T>(val));
  }

};


InsertScan::~InsertScan() {
}

storage::atable_ptr_t InsertScan::buildFromJson() {

  auto result = input.getTable()->copy_structure_modifiable();
  result->resize(_raw_data.size());

  set_value_functor fun(result);
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
  std::shared_ptr<const Store> store = std::dynamic_pointer_cast<const Store>(input.getTable(0));
  if (!store) {
    throw std::runtime_error("Insert without delta is not supported");
  }

  if (!_data)
    _data = buildFromJson();

  size_t max = store->getDeltaTable()->size();
  store->getDeltaTable()->resize(max + _data->size());

  for(size_t i=0, upper = _data->size(); i < upper; ++i) {
    store->getDeltaTable()->copyRowFrom(_data, i, max+i, true);
  }

  addResult(input.getTable(0));
}

void InsertScan::setInputData(const storage::atable_ptr_t &c) {
  _data = c;
}

std::shared_ptr<_PlanOperation> InsertScan::parse(Json::Value &data) {
  auto result = std::make_shared<InsertScan>();

  if (data.isMember("data")) {
    result->_raw_data = collect(data["data"], [](Json::Value& v){
      return collect(v, [](Json::Value& c){ return c; });
    });
  }
  return result;
}

}
}
