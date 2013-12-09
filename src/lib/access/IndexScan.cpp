// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/IndexScan.h"

#include <memory>

#include "access/system/BasicParser.h"
#include "access/json_converters.h"
#include "access/system/QueryParser.h"

#include "io/StorageManager.h"

#include "storage/InvertedIndex.h"
#include "storage/meta_storage.h"
#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

struct CreateIndexValueFunctor {
  typedef AbstractIndexValue *value_type;

  const Json::Value &_d;

  explicit CreateIndexValueFunctor(const Json::Value &c): _d(c) {}

  template<typename R>
  value_type operator()() {
    IndexValue<R> *v = new IndexValue<R>();
    v->value = json_converter::convert<R>(_d["value"]);
    return v;
  }
};

struct ScanIndexFunctor {
  typedef storage::pos_list_t *value_type;

  std::shared_ptr<storage::AbstractIndex> _index;
  AbstractIndexValue *_indexValue;

  ScanIndexFunctor(AbstractIndexValue *i, std::shared_ptr<storage::AbstractIndex> d):
    _index(d), _indexValue(i) {}

  template<typename ValueType>
  value_type operator()() {
    auto idx = std::dynamic_pointer_cast<storage::InvertedIndex<ValueType>>(_index);
    auto v = static_cast<IndexValue<ValueType>*>(_indexValue);
    storage::pos_list_t *result = new storage::pos_list_t(idx->getPositionsForKey(v->value));
    return result;
  }
};

namespace {
  auto _ = QueryParser::registerPlanOperation<IndexScan>("IndexScan");
}

IndexScan::~IndexScan() {
  delete _value;
}

void IndexScan::executePlanOperation() {
  auto sm = io::StorageManager::getInstance();
  auto idx = sm->getInvertedIndex(_indexName);

  // Handle type of index and value
  storage::type_switch<hyrise_basic_types> ts;
  ScanIndexFunctor fun(_value, idx);
  storage::pos_list_t *pos = ts(input.getTable(0)->typeOfColumn(_field_definition[0]), fun);

  addResult(storage::PointerCalculator::create(input.getTable(0), pos));
}

std::shared_ptr<PlanOperation> IndexScan::parse(const Json::Value &data) {
  std::shared_ptr<IndexScan> s = BasicParser<IndexScan>::parse(data);
  storage::type_switch<hyrise_basic_types> ts;
  CreateIndexValueFunctor civf(data);
  s->_value = ts(data["vtype"].asUInt(), civf);
  s->_indexName = data["index"].asString();
  return s;
}

const std::string IndexScan::vname() {
  return "IndexScan";
}

void IndexScan::setIndexName(const std::string &name) {
  _indexName = name;
}

namespace {
  auto _2 = QueryParser::registerPlanOperation<MergeIndexScan>("MergeIndexScan");
}

MergeIndexScan::~MergeIndexScan() {
}

void MergeIndexScan::executePlanOperation() {
  auto left = std::dynamic_pointer_cast<const storage::PointerCalculator>(input.getTable(0));
  auto right = std::dynamic_pointer_cast<const storage::PointerCalculator>(input.getTable(1));

  storage::pos_list_t result(std::max(left->getPositions()->size(), right->getPositions()->size()));

  auto it = std::set_intersection(left->getPositions()->begin(),
                                  left->getPositions()->end(),
                                  right->getPositions()->begin(),
                                  right->getPositions()->end(),
                                  result.begin());

  auto tmp = storage::PointerCalculator::create(left->getActualTable(), new storage::pos_list_t(result.begin(), it));
  addResult(tmp);
}

std::shared_ptr<PlanOperation> MergeIndexScan::parse(const Json::Value &data) {
  return BasicParser<MergeIndexScan>::parse(data);
}

const std::string MergeIndexScan::vname() {
  return "MergeIndexScan";
}

}
}
