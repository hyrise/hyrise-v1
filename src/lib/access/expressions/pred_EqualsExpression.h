// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <algorithm>
#include <functional>
#include <limits>

#include "helper/types.h"
#include "helper/checked_cast.h"
#include "helper/make_unique.h"
#include "storage/column_extract.h"
#include "pred_common.h"

namespace hyrise {
namespace access {

template <typename T, template <typename U> class Operator>
class OpExpression : public SimpleFieldExpression {
 private:
  using operator_type = OpExpression<T, Operator>;
  Operator<value_id_t> _vid_operator;
  Operator<T> _value_operator;

 public:
  T value;

  OpExpression(size_t _input, field_t _field, T _value) : SimpleFieldExpression(_input, _field), value(_value) {}
  OpExpression(size_t _input, field_name_t _field, T _value) : SimpleFieldExpression(_input, _field), value(_value) {}
  OpExpression(storage::c_atable_ptr_t& _table, field_t _field, T _value)
      : SimpleFieldExpression(_table, _field), value(_value) {}

  virtual std::unique_ptr<AbstractExpression> clone() { return make_unique<operator_type>(table, field, value); }

  virtual bool operator()(size_t row) { return _value_operator(table->getValue<T>(field, row), value); }


  std::function<value_id_t(T)> retrieveVid(storage::BaseDictionary<T>& dict, std::equal_to<value_id_t>&) {
    return std::bind(&storage::BaseDictionary<T>::getValueIdForValue, &dict, std::placeholders::_1);
  }

  std::function<value_id_t(T)> retrieveVid(storage::BaseDictionary<T>& dict, std::less<value_id_t>&) {
    return std::bind(&storage::BaseDictionary<T>::getLowerBoundValueIdForValue, &dict, std::placeholders::_1);
  }

  std::function<value_id_t(T)> retrieveVid(storage::BaseDictionary<T>& dict, std::greater<value_id_t>&) {
    return std::bind(&storage::BaseDictionary<T>::getUpperBoundValueIdForValue, &dict, std::placeholders::_1);
  }

  std::function<value_id_t(T)> retrieveVid(storage::BaseDictionary<T>& dict, std::less_equal<value_id_t>&) {
    return std::bind(&storage::BaseDictionary<T>::getUpperBoundValueIdForValue, &dict, std::placeholders::_1);
  }

  std::function<value_id_t(T)> retrieveVid(storage::BaseDictionary<T>& dict, std::greater_equal<value_id_t>&) {
    return std::bind(&storage::BaseDictionary<T>::getLowerBoundValueIdForValue, &dict, std::placeholders::_1);
  }

  inline void op(const storage::BaseAttributeVector<value_id_t>& aav,
                 size_t col,
                 size_t av_pos,
                 size_t abs_pos,
                 value_id_t vid,
                 storage::pos_list_t& results) {
    if (_vid_operator(aav.get(col, av_pos), vid)) {
      results.push_back(abs_pos);
    }
  }

  inline void op_unordered(const storage::BaseAttributeVector<value_id_t>& aav,
                           const storage::BaseDictionary<T>& dict,
                           size_t col,
                           size_t av_pos,
                           size_t abs_pos,
                           const T& value,
                           storage::pos_list_t& results) {
    if (_value_operator(dict.getValueForValueId(aav.get(col, av_pos)), value)) {
      results.push_back(abs_pos);
    }
  }


  virtual storage::pos_list_t matchAll(storage::pos_list_t& pos) {
    storage::pos_list_t results;
    if (pos.size() == 0)
      return results;
    // Assuming that position list is ordered, we can extract min and max
    auto start = pos.front();
    auto stop = pos.back();

    auto parts = storage::column_parts_extract(*table.get(), field, start, stop);
    assert(parts.size() != 0);
    auto it = pos.begin();
    auto et = pos.end();
    parts.remove_if([](decltype(*parts.begin()) & part) { return part.iterStart == part.iterEnd; });
    for (const auto& part : parts) {
      const auto dict = checked_pointer_cast<storage::BaseDictionary<T>>(part.table.dictionaryAt(part.columnOffset));
      const auto vid = retrieveVid (*dict.get(), _vid_operator)(value);
      const auto aav = checked_pointer_cast<storage::BaseAttributeVector<value_id_t>>(
          part.table.getAttributeVectors(part.columnOffset).at(0).attribute_vector);
      if (dict->isOrdered()) {
        do {
          auto p = *it - part.verticalStart;
          op(*aav.get(), part.columnOffset, p, part.verticalStart + p, vid, results);
        } while ((*(++it) < part.verticalEnd) && (it != et));
      } else {
        do {
          auto p = *it - part.verticalStart;
          op_unordered(*aav.get(), *dict.get(), part.columnOffset, p, part.verticalStart + p, value, results);
        } while ((*(++it) < part.verticalEnd) && (it != et));
      }
    }
    return results;
  }

  virtual storage::pos_list_t matchAll(size_t start, size_t stop) {
    auto parts = storage::column_parts_extract(*table.get(), field, start, stop);
    assert(parts.size() != 0);
    storage::pos_list_t results;
    parts.remove_if([](decltype(*parts.begin()) & part) { return part.iterStart == part.iterEnd; });
    for (const auto& part : parts) {
      const auto dict = checked_pointer_cast<storage::BaseDictionary<T>>(part.table.dictionaryAt(part.columnOffset));
      const auto vid = retrieveVid (*dict.get(), _vid_operator)(value);
      const auto aav = checked_pointer_cast<storage::BaseAttributeVector<value_id_t>>(
          part.table.getAttributeVectors(part.columnOffset).at(0).attribute_vector);
      if (dict->isOrdered()) {
        for (std::size_t p = part.iterStart, e = part.iterEnd; p < e; p++) {
          op(*aav.get(), part.columnOffset, p, part.verticalStart + p, vid, results);
        }
      } else {
        for (std::size_t p = part.iterStart, e = part.iterEnd; p < e; p++) {
          op_unordered(*aav.get(), *dict.get(), part.columnOffset, p, part.verticalStart + p, value, results);
        }
      }
      start = part.verticalEnd + 1;
    }
    return results;
  }
};

template <typename T>
using EqualsExpression = OpExpression<T, std::equal_to>;

template <typename T>
using LessExpression = OpExpression<T, std::less>;

template <typename T>
using LessEqualExpression = OpExpression<T, std::less_equal>;

template <typename T>
using GreaterExpression = OpExpression<T, std::greater>;

template <typename T>
using GreaterEqualExpression = OpExpression<T, std::greater_equal>;
}
}  // namespace hyrise::access
