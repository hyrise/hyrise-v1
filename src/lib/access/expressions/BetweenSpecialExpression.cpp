// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_BETWEENSPECIALEXPRESSION_HPP_
#define SRC_LIB_ACCESS_BETWEENSPECIALEXPRESSION_HPP_

#include "json.h"

#include "helper/make_unique.h"
#include "helper/types.h"

#include "access/expressions/AbstractExpression.h"
#include "access/expressions/ExpressionRegistration.h"

#include "storage/AbstractTable.h"
#include "storage/FixedLengthVector.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/storage_types.h"

namespace hyrise { namespace access {

template <typename T>
class BetweenSpecialExpression : public AbstractExpression {
  storage::c_atable_ptr_t _table;
  std::shared_ptr<FixedLengthVector<value_id_t>> _vector;
  std::shared_ptr<OrderPreservingDictionary<T>> _dict;
  const size_t _column;
  const T _fromValue;
  const T _toValue;
  value_id_t _fromValueid;
  value_id_t _toValueid;
 public:

  explicit BetweenSpecialExpression(
    const size_t& column, 
    const T& fromValue, 
    const T& toValue) : 
      _column(column),
      _fromValue(fromValue),
      _toValue(toValue) {};

  inline bool operator()(const size_t& row) { return _fromValueid <= _vector->getRef(_column, row) && _vector->getRef(_column, row) <= _toValueid; }

  virtual pos_list_t* match(const size_t start, const size_t stop) {
    // if from-to-range is out of bounds for dictionary
    if (_fromValue > _dict->getGreatestValue() || _toValue < _dict->getSmallestValue()) { 
      return new pos_list_t();
    } // select everything
    else if (_fromValue < _dict->getSmallestValue() && _toValue > _dict->getGreatestValue()) {
      pos_list_t* l = new pos_list_t(stop-start);
      // fill with all positions in [start, stop)
      std::iota(l->begin(), l->end(), start);
      return l;
    }

    // find an inclusive range
    // if border values do not exist, chose the next "more inner" ones
    _fromValueid = _dict->getValueIdForValueGreaterOrEqualTo(_fromValue);
    _toValueid = _dict->getValueIdForValueSmallerOrEqualTo(_toValue);

    auto pl = new pos_list_t;
    for(size_t row=start; row < stop; ++row) {
      if (this->BetweenSpecialExpression::operator()(row)) {
        pl->push_back(row);
      }
    }
    return pl;
  }


  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t> &tables) {
    _table = tables.at(0);
    const auto& avs = _table->getAttributeVectors(_column);
    _vector = std::dynamic_pointer_cast<FixedLengthVector<value_id_t>>(avs.at(0).attribute_vector);
    _dict = std::dynamic_pointer_cast<OrderPreservingDictionary<T>>(_table->dictionaryAt(_column));
    if (!(_vector && _dict)) throw std::runtime_error("Could not extract proper structures");
  }

  static std::unique_ptr<BetweenSpecialExpression<T>> parse(const Json::Value& data);

};

template<> std::unique_ptr<BetweenSpecialExpression<hyrise_int_t>> BetweenSpecialExpression<hyrise_int_t>::parse(const Json::Value& data) {
      return make_unique<BetweenSpecialExpression<hyrise_int_t>>(
      data["column"].asUInt(),
      data["fromValue"].asInt(),
      data["toValue"].asInt());
};

template<> std::unique_ptr<BetweenSpecialExpression<hyrise_float_t>> BetweenSpecialExpression<hyrise_float_t>::parse(const Json::Value& data) {
      return make_unique<BetweenSpecialExpression<hyrise_float_t>>(
      data["column"].asUInt(),
      data["fromValue"].asDouble(),
      data["toValue"].asDouble());
};

template<> std::unique_ptr<BetweenSpecialExpression<hyrise_string_t>> BetweenSpecialExpression<hyrise_string_t>::parse(const Json::Value& data) {
      return make_unique<BetweenSpecialExpression<hyrise_string_t>>(
      data["column"].asUInt(),
      data["fromValue"].asString(),
      data["toValue"].asString());
};

namespace {
  auto _1 = Expressions::add<BetweenSpecialExpression<hyrise_int_t>>("hyrise::intbetween");
  auto _2 = Expressions::add<BetweenSpecialExpression<hyrise_float_t>>("hyrise::floatbetween");
  auto _3 = Expressions::add<BetweenSpecialExpression<hyrise_string_t>>("hyrise::stringbetween");
}


}}

#endif
