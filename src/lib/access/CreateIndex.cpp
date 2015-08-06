// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/CreateIndex.h"

#include <string>
#include <vector>
#include <map>

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"

#include "io/StorageManager.h"

#include "storage/AbstractTable.h"
#include "storage/meta_storage.h"
#include "storage/storage_types.h"
#include "storage/PointerCalculator.h"
#include "storage/AbstractIndex.h"
#include "storage/InvertedIndex.h"
#include "storage/Store.h"

namespace hyrise {
namespace access {

struct CreateIndexFunctor {
  typedef std::shared_ptr<storage::AbstractIndex> value_type;
  const storage::c_atable_ptr_t& in;
  size_t column;
  const std::string& index_name;

  CreateIndexFunctor(const storage::c_atable_ptr_t& t, size_t c, std::string& index_name)
      : in(t), column(c), index_name(index_name) {}

  template <typename R>
  value_type operator()() {
    return std::make_shared<storage::InvertedIndex<R>>(in, column, index_name);
  }
};

namespace {
auto _ = QueryParser::registerPlanOperation<CreateIndex>("CreateIndex");
}

CreateIndex::~CreateIndex() {}

void CreateIndex::executePlanOperation() {
  const auto& in = input.getTable(0);
  std::shared_ptr<storage::AbstractIndex> _index;
  auto column = _field_definition[0];
  auto c_store = std::dynamic_pointer_cast<const storage::Store>(in);
  auto store = std::const_pointer_cast<storage::Store>(c_store);

  CreateIndexFunctor fun(in, column, _index_name);
  storage::type_switch<hyrise_basic_types> ts;
  _index = ts(in->typeOfColumn(column), fun);

  auto sm = io::StorageManager::getInstance();
  sm->addInvertedIndex(_index_name, _index);

  if (store)
    store->addMainIndex(_index, _field_definition);
}

std::shared_ptr<PlanOperation> CreateIndex::parse(const Json::Value& data) {
  auto i = BasicParser<CreateIndex>::parse(data);
  i->setIndexName(data["index_name"].asString());
  return i;
}

void CreateIndex::setIndexName(const std::string& t) { _index_name = t; }
}
}
