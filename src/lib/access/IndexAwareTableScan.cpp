// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/IndexAwareTableScan.h"

#include <memory>

#include "access/system/BasicParser.h"
#include "access/json_converters.h"
#include "access/system/QueryParser.h"

#include "io/StorageManager.h"
#include "io/TransactionManager.h"

#include "storage/InvertedIndex.h"
#include "storage/Store.h"
#include "storage/meta_storage.h"
#include "storage/PointerCalculator.h"
#include "storage/GroupkeyIndex.h"
#include "storage/DeltaIndex.h"
#include "storage/storage_types.h"

#include "helper/checked_cast.h"
#include "helper/PositionsIntersect.h"

#include "access/expressions/pred_buildExpression.h"
#include "access/expressions/pred_GreaterThanExpression.h"
#include "access/expressions/pred_EqualsExpression.h"
#include "access/expressions/pred_LessThanExpression.h"
#include "access/expressions/pred_CompoundExpression.h"
#include "access/expressions/expression_types.h"
#include "access/IntersectPositions.h"
#include "access/SimpleTableScan.h"
#include "access/system/ResponseTask.h"

#include <chrono>

#define EXPR_SPECIFIC(EXPR, COMP, TYPE)                                                                 \
  {                                                                                                     \
    auto e = dynamic_cast<GenericExpressionValue<TYPE, COMP<TYPE>>*>(c);                                \
    if (e) {                                                                                            \
      GroupkeyIndexFunctor groupkey_functor_main(Json::Value(e->value),                                 \
                                                 "mcidx__" + _tableName + "__main__" + e->field_name,   \
                                                 PredicateType::EXPR##Value,                            \
                                                 e->field_name);                                        \
      _idx_functors_main.push_back(groupkey_functor_main);                                              \
      GroupkeyIndexFunctor groupkey_functor_delta(Json::Value(e->value),                                \
                                                  "mcidx__" + _tableName + "__delta__" + e->field_name, \
                                                  PredicateType::EXPR##Value,                           \
                                                  e->field_name);                                       \
      _idx_functors_delta.push_back(groupkey_functor_delta);                                            \
      assert(!e->field_name.empty());                                                                   \
      addField(e->field_name);                                                                          \
      break;                                                                                            \
    }                                                                                                   \
  };

#define EXPR_SPECIFIC_ALL_TYPES(EXPR, COMP)  \
  EXPR_SPECIFIC(EXPR, COMP, hyrise_int_t);   \
  EXPR_SPECIFIC(EXPR, COMP, hyrise_float_t); \
  EXPR_SPECIFIC(EXPR, COMP, hyrise_string_t);

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<IndexAwareTableScan>("IndexAwareTableScan");
}

IndexAwareTableScan::IndexAwareTableScan() : _performValidation(false) {}

IndexAwareTableScan::~IndexAwareTableScan() {}

struct GroupkeyIndexFunctor {
  typedef storage::PositionRange value_type;

  std::string _indexname;
  Json::Value _indexValue1;
  Json::Value _indexValue2;
  PredicateType::type _predicate_type;
  std::string _fieldname;
  size_t _column;
  bool _flagged_for_removal;
  std::shared_ptr<storage::AbstractIndex> _index;

  GroupkeyIndexFunctor(Json::Value indexValue1,
                       std::string indexname,
                       PredicateType::type predicate_type,
                       std::string fieldname)
      : _indexname(indexname),
        _indexValue1(indexValue1),
        _predicate_type(predicate_type),
        _fieldname(fieldname),
        _flagged_for_removal(false) {}

  template <typename ValueType>
  value_type operator()() {
    if (auto idx_main = std::dynamic_pointer_cast<storage::GroupkeyIndex<ValueType>>(_index))
      return callIndex<ValueType>(idx_main);
    else if (auto idx_delta = std::dynamic_pointer_cast<storage::DeltaIndex<ValueType>>(_index)) {
      return callIndex<ValueType>(idx_delta);
    } else
      throw std::runtime_error("IndexAwareTable scan only supports GroupKeyIndex and DeltaIndex: " + _indexname);
  }

  template <typename ValueType, typename IndexType>
  value_type callIndex(const IndexType& idx) {
    ValueType v1 = json_converter::convert<ValueType>(_indexValue1);
    ValueType v2 = json_converter::convert<ValueType>(_indexValue2);
    switch (_predicate_type) {
      case PredicateType::EqualsExpressionValue:
        return idx->getPositionsForKey(v1);
        break;
      case PredicateType::GreaterThanExpressionValue:
        return idx->getPositionsForKeyGT(v1);
        break;
      case PredicateType::GreaterThanEqualsExpressionValue:
        return idx->getPositionsForKeyGTE(v1);
        break;
      case PredicateType::LessThanExpressionValue:
        return idx->getPositionsForKeyLT(v1);
        break;
      case PredicateType::LessThanEqualsExpressionValue:
        return idx->getPositionsForKeyLTE(v1);
        break;
      case PredicateType::BetweenExpression:
        return idx->getPositionsForKeyBetween(v1, v2);
        break;
      default:
        throw std::runtime_error("Unsupported predicate type in IndexAwareTableScan");
    }
  }
};

void IndexAwareTableScan::_getIndexResults(std::shared_ptr<const storage::Store> t_store,
                                           pos_list_t*& result,
                                           std::vector<GroupkeyIndexFunctor>& functors) {

  // calculate the index results
  std::vector<storage::PositionRange> idx_results;
  storage::type_switch<hyrise_basic_types> ts;
  std::vector<std::shared_ptr<storage::AbstractIndex>> indices;

  indices.reserve(functors.size());
  idx_results.reserve(functors.size());

  // sort functors after name, so locking is deterministic
  std::sort(begin(functors), end(functors), [](const GroupkeyIndexFunctor& a, const GroupkeyIndexFunctor& b) {
    return a._indexname < b._indexname;
  });

  for (auto functor : functors) {
    auto index = io::StorageManager::getInstance()->getInvertedIndex(functor._indexname, true);
    indices.push_back(index);
    functor._index = index;
    size_t column = t_store->numberOfColumn(functor._fieldname);
    auto idx_result = ts(t_store->typeOfColumn(column), functor);
    idx_results.push_back(idx_result);
  }

  // sort so we start with the smallest range
  if (idx_results.size() > 2) {
    std::sort(begin(idx_results),
              end(idx_results),
              [](const storage::PositionRange& a, const storage::PositionRange& b) { return a.size() < b.size(); });
  }

  // result is not going to contain more elements than the smallest, so start with that one
  auto r = idx_results[0];
  pos_list_t* tmp_result = new pos_list_t(r.size());
  pos_list_t* swap_tmp = nullptr;
  std::copy(r.cbegin(), r.cend(), tmp_result->begin());
  if (!r.isSorted()) {
    std::sort(tmp_result->begin(), tmp_result->end());
  }

  // if desired, validate now. validation is only necessary for the first list
  if (_performValidation) {
    for (auto it = tmp_result->cbegin(); it != tmp_result->cend(); ++it) {
      if (t_store->isVisibleForTransaction(*it, _txContext.lastCid, _txContext.tid)) {
        result->push_back(*it);
      }
    }
    swap_tmp = result;
    result = tmp_result;
    tmp_result = swap_tmp;
    result->clear();
  }

  // if we only have one idx result, this is the final one
  if (idx_results.size() == 1) {
    swap_tmp = result;
    result = tmp_result;
    tmp_result = swap_tmp;
  }
  // otherwise we intersect the first two results
  else if (idx_results.size() >= 2) {
    result->reserve(tmp_result->size());
    auto a = storage::PositionRange(tmp_result->begin(), tmp_result->end(), true);
    auto b = idx_results[1];
    pos_list_t* b_sorted = nullptr;

    if (!b.isSorted()) {
      // copy and sort
      b_sorted = new pos_list_t(b.size());
      std::copy(b.cbegin(), b.cend(), b_sorted->begin());
      std::sort(b_sorted->begin(), b_sorted->end());
      b = storage::PositionRange(b_sorted->begin(), b_sorted->end(), true);
    }

    intersect_pos_list(a.cbegin(), a.cend(), b.cbegin(), b.cend(), std::back_inserter(*result));
    delete b_sorted;
    tmp_result->clear();
  }

  // if we have more than two results, intersect them too
  if (idx_results.size() > 2 && !result->empty()) {
    pos_list_t* idx_result_sorted = new pos_list_t;  // nullptr;
    auto it = idx_results.begin();
    ++it;
    ++it;
    auto it_end = idx_results.end();

    for (; it != it_end; ++it) {
      if (result->empty())
        break;  // when result is empty, final intersect is empty too
      if (!it->isSorted()) {
        // copy and sort
        idx_result_sorted->resize(it->size());
        std::copy(it->cbegin(), it->cend(), idx_result_sorted->begin());
        std::sort(idx_result_sorted->begin(), idx_result_sorted->end());
        intersect_pos_list(idx_result_sorted->begin(),
                           idx_result_sorted->end(),
                           result->begin(),
                           result->end(),
                           std::back_inserter(*tmp_result));
        idx_result_sorted->clear();
      } else {
        storage::PositionRange tmp_range(result->begin(), result->end(), true);
        intersect_pos_list(
            it->cbegin(), it->cend(), tmp_range.cbegin(), tmp_range.cend(), std::back_inserter(*tmp_result));
      }
      swap_tmp = result;
      result = tmp_result;
      tmp_result = swap_tmp;
      tmp_result->clear();
    }
    delete idx_result_sorted;
  }

  delete tmp_result;
}

void IndexAwareTableScan::_consolidateFunctors(std::shared_ptr<const storage::Store> t_store,
                                               std::vector<GroupkeyIndexFunctor>& functors) {
  // set columns
  for (auto& functor : functors)
    functor._column = t_store->numberOfColumn(functor._fieldname);

  std::sort(begin(functors), end(functors), [](const GroupkeyIndexFunctor& a, const GroupkeyIndexFunctor& b) {
    return (a._column) < (b._column);
  });

  // consolidate LT and GT on same column to between expression
  size_t funct_size_minus_one = functors.size() - 1;
  for (size_t i = 0; i < funct_size_minus_one; ++i) {
    if (functors[i]._column == functors[i + 1]._column) {
      if (functors[i]._predicate_type == PredicateType::GreaterThanEqualsExpressionValue &&
          functors[i + 1]._predicate_type == PredicateType::LessThanEqualsExpressionValue) {
        functors[i + 1]._predicate_type = PredicateType::BetweenExpression;
        functors[i + 1]._indexValue2 = functors[i + 1]._indexValue1;
        functors[i + 1]._indexValue1 = functors[i]._indexValue1;
        functors[i]._flagged_for_removal = true;
      } else if (functors[i + 1]._predicate_type == PredicateType::GreaterThanEqualsExpressionValue &&
                 functors[i]._predicate_type == PredicateType::LessThanEqualsExpressionValue) {
        functors[i + 1]._predicate_type = PredicateType::BetweenExpression;
        functors[i + 1]._indexValue2 = functors[i]._indexValue1;
        functors[i]._flagged_for_removal = true;
      }
    }
  }

  // delete consolidated functors
  functors.erase(std::remove_if(functors.begin(), functors.end(), [](const GroupkeyIndexFunctor& f) {
                   return f._flagged_for_removal;
                 }),
                 functors.end());
}

void IndexAwareTableScan::executePlanOperation() {
  // auto start = std::chrono::system_clock::now();
  auto t = input.getTables()[0];
  auto t_store = std::dynamic_pointer_cast<const storage::Store>(t);

  // we only work on stores
  if (!t_store)
    throw std::runtime_error("IndexAwareTableScan only works on stores");

  // consolidate functors, basically rewriting GT and LT on the same column to BETWEEN
  this->_consolidateFunctors(t_store, _idx_functors_main);
  this->_consolidateFunctors(t_store, _idx_functors_delta);

  // get main and delta results from indices
  pos_list_t* main_result = new pos_list_t;
  pos_list_t* delta_result = new pos_list_t;
  this->_getIndexResults(t_store, main_result, _idx_functors_main);
  this->_getIndexResults(t_store, delta_result, _idx_functors_delta);

  // union results
  main_result->reserve(main_result->size() + delta_result->size());
  std::copy(delta_result->begin(), delta_result->end(), std::back_inserter(*main_result));
  addResult(storage::PointerCalculator::create(t, main_result));

  delete delta_result;
}


void IndexAwareTableScan::setFunctorsFromInfo(const std::string& indexname_main,
                                              const std::string& indexname_delta,
                                              std::string fieldname,
                                              Json::Value value) {
  /*
  Fast-Track initiziliaztion for TPC-C NewOrder Procedure.
  */
  GroupkeyIndexFunctor groupkey_functor_main(value, indexname_main, PredicateType::EqualsExpressionValue, fieldname);
  GroupkeyIndexFunctor groupkey_functor_delta(value, indexname_delta, PredicateType::EqualsExpressionValue, fieldname);
  this->_idx_functors_main.push_back(groupkey_functor_main);
  this->_idx_functors_delta.push_back(groupkey_functor_delta);
}



std::shared_ptr<PlanOperation> IndexAwareTableScan::parse(const Json::Value& data) {

  std::shared_ptr<IndexAwareTableScan> idx_scan = BasicParser<IndexAwareTableScan>::parse(data);

  if (!data.isMember("predicates")) {
    throw std::runtime_error("There is no reason for a IndexAwareScan without predicates");
  }
  if (!data.isMember("tablename")) {
    throw std::runtime_error("IndexAwareScan needs a base table name");
  }

  idx_scan->_setPredicateFromJson(buildExpression(data["predicates"]));

  PredicateType::type pred_type;
  std::string tablename = data["tablename"].asString();

  for (unsigned i = 0; i < data["predicates"].size(); ++i) {
    const Json::Value& predicate = data["predicates"][i];
    pred_type = parsePredicateType(predicate["type"]);

    if (pred_type == PredicateType::AND)
      continue;

    if (pred_type != PredicateType::EqualsExpressionValue && pred_type != PredicateType::LessThanExpressionValue &&
        pred_type != PredicateType::GreaterThanExpressionValue &&
        pred_type != PredicateType::LessThanEqualsExpressionValue &&
        pred_type != PredicateType::GreaterThanEqualsExpressionValue)
      throw std::runtime_error("IndexAwareScan: Unsupported predicate type");

    if (predicate["f"].isNumeric())
      throw std::runtime_error("For now, IndexAwareScan requires fields to be specified via fieldnames");

    std::string fieldname = predicate["f"].asString();
    std::string indexname_main = "mcidx__" + tablename + "__main__" + fieldname;
    std::string indexname_delta = "mcidx__" + tablename + "__delta__" + fieldname;

    GroupkeyIndexFunctor groupkey_functor_main(predicate["value"], indexname_main, pred_type, fieldname);
    GroupkeyIndexFunctor groupkey_functor_delta(predicate["value"], indexname_delta, pred_type, fieldname);
    idx_scan->_idx_functors_main.push_back(groupkey_functor_main);
    idx_scan->_idx_functors_delta.push_back(groupkey_functor_delta);
  }

  return idx_scan;
}

const std::string IndexAwareTableScan::vname() { return "IndexAwareTableScan"; }

void IndexAwareTableScan::_setPredicateFromJson(AbstractExpression* c) {
  SimpleExpression* se = dynamic_cast<SimpleExpression*>(c);
  if (!se)
    throw std::runtime_error("Expression not parsable");

  _predicate = se;
}

void IndexAwareTableScan::setPredicate(AbstractExpression* c) {
  assert(!_tableName.empty());

  SimpleExpression* se = dynamic_cast<SimpleExpression*>(c);
  if (!se)
    throw std::runtime_error("Expression not parsable");

  CompoundExpression* ce = dynamic_cast<CompoundExpression*>(c);
  if (ce != nullptr) {
    if (ce->type != ExpressionType::AND)
      throw std::runtime_error("IndexAwareScan only supports AND for CompoundExpressions");
    // this is not a real limitation - it's just that we haven't implemented the rest yet.
    setPredicate(ce->lhs);
    setPredicate(ce->rhs);
    _predicate = se;
    return;
  }

  _predicate = se;

  do {
    EXPR_SPECIFIC_ALL_TYPES(LessThanExpression, std::less);
    EXPR_SPECIFIC_ALL_TYPES(LessThanEqualsExpression, std::less_equal);
    EXPR_SPECIFIC_ALL_TYPES(EqualsExpression, std::equal_to);
    EXPR_SPECIFIC_ALL_TYPES(GreaterThanExpression, std::greater);
    EXPR_SPECIFIC_ALL_TYPES(GreaterThanEqualsExpression, std::greater_equal);

    throw std::runtime_error("Expression not recognized");
  } while (false);
}

void IndexAwareTableScan::setTableName(const std::string& name) { _tableName = name; }

void IndexAwareTableScan::setPerformValidation(bool validate) { _performValidation = validate; }

}  // namespace access
}  // namespace hyrise
