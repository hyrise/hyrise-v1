// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "pred_common.h"

namespace hyrise {
namespace access {

template <typename T>
class LessThanExpression : public SimpleFieldExpression {
 private:
  ValueId lower_bound;
  T value;
  std::shared_ptr<storage::BaseDictionary<T>> valueIdMap;
  bool value_exists;

 public:

  LessThanExpression(size_t i, field_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  LessThanExpression(size_t i, field_name_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  LessThanExpression(storage::c_atable_ptr_t _table, field_t _field, T _value) :
      SimpleFieldExpression(_table, _field), value(_value)
  {}

  virtual void walk(const std::vector<storage::c_atable_ptr_t > &l) {

    SimpleFieldExpression::walk(l);
    valueIdMap = std::dynamic_pointer_cast<storage::BaseDictionary<T>>(table->dictionaryAt(field));
    lower_bound.table = 0;
    lower_bound.valueId = valueIdMap->getValueIdForValue(value);
    value_exists = valueIdMap->isValueIdValid(lower_bound.valueId) && value == valueIdMap->getValueForValueId(lower_bound.valueId);
  }

  virtual ~LessThanExpression() { }

  inline virtual bool operator()(size_t row) {
    ValueId valueId = table->getValueId(field, row);
    if (valueId.valueId < lower_bound.valueId) {
      return true;
    } else
      return false;
  }
};


template <typename T>
class LessThanExpressionRaw : public SimpleFieldExpression {
 private:
  T value;

 public:

  LessThanExpressionRaw(size_t i, field_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  LessThanExpressionRaw(size_t i, field_name_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  LessThanExpressionRaw(const storage::c_atable_ptr_t& _table, field_t _field, T _value) :
      SimpleFieldExpression(_table, _field), value(_value)
  {}


  virtual ~LessThanExpressionRaw() { }

  inline virtual bool operator()(size_t row) {
    return (std::dynamic_pointer_cast<const storage::RawTable>(table))->template getValue<T>(field, row) < value;
  }
};

} } // pragma once

