// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "pred_common.h"

namespace hyrise {
namespace access {

template <typename T>
class GreaterThanExpression : public SimpleFieldExpression {
 private:
  ValueId lower_bound;
  T value;
  std::shared_ptr<storage::BaseDictionary<T>> valueIdMap;
  bool value_exists;

 public:

  GreaterThanExpression(size_t i, field_t f, T v):
      SimpleFieldExpression(i, f), value(v)
  {}

  GreaterThanExpression(size_t i, field_name_t f, T v):
      SimpleFieldExpression(i, f), value(v)
  {}

  GreaterThanExpression(storage::c_atable_ptr_t _table, field_t _field, T _value) : SimpleFieldExpression(_table, _field), value(_value)
  {}

  virtual ~GreaterThanExpression() { }

  virtual void walk(const std::vector<storage::c_atable_ptr_t > &l) {
    SimpleFieldExpression::walk(l);

    valueIdMap = std::dynamic_pointer_cast<storage::BaseDictionary<T>>(table->dictionaryAt(field));
    lower_bound.table = 0;
    lower_bound.valueId = valueIdMap->getValueIdForValue(value);
    value_exists = valueIdMap->isValueIdValid(lower_bound.valueId) &&
        value == valueIdMap->getValueForValueId(lower_bound.valueId);
  }

  inline virtual bool operator()(size_t row) {
    ValueId valueId = table->getValueId(field, row);

    if (valueId.table == lower_bound.table) {
      if (valueId.valueId > lower_bound.valueId) {
        return true;
      }

      if (valueId.valueId < lower_bound.valueId) {
        return false;
      }

      if (value_exists) {
        return false;
      }
    }

    return table->getValue<T>(field, row) > value;
  }
};



template <typename T>
class GreaterThanExpressionRaw : public SimpleFieldExpression {
 private:
  T value;

 public:

  GreaterThanExpressionRaw(size_t i, field_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  GreaterThanExpressionRaw(size_t i, field_name_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  GreaterThanExpressionRaw(const storage::c_atable_ptr_t& _table, field_t _field, T _value) :
      SimpleFieldExpression(_table, _field), value(_value)
  {}


  virtual ~GreaterThanExpressionRaw() { }

  inline virtual bool operator()(size_t row) {
    return (std::dynamic_pointer_cast<const storage::RawTable>(table))->template getValue<T>(field, row) > value;
  }
};

} } // namespace hyrise::access

