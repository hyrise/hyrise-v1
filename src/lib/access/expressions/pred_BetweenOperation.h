// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "helper/types.h"
#include "pred_common.h"

namespace hyrise {
namespace access {

template <typename T>
class BetweenExpression : public SimpleFieldExpression {
 private:
  ValueId lower_bound;
  ValueId upper_bound;
  T lower_value;
  T upper_value;
  std::shared_ptr<storage::BaseDictionary<T>> valueIdMap;
  bool lower_value_exists;
  bool upper_value_exists;
 public:

  BetweenExpression(size_t i, field_t f, T _lower_value, T _upper_value):
      SimpleFieldExpression(i, f), lower_value(_lower_value), upper_value(_upper_value)
  {}

  BetweenExpression(size_t i, field_name_t f, T _lower_value, T _upper_value):
      SimpleFieldExpression(i, f), lower_value(_lower_value), upper_value(_upper_value)
  {}

  BetweenExpression(storage::c_atable_ptr_t _table, field_t _field, T _lower_value, T _upper_value) :
      SimpleFieldExpression(_table, _field), lower_value(_lower_value), upper_value(_upper_value)
  {}

  virtual void walk(const std::vector<storage::c_atable_ptr_t > &l) {
    SimpleFieldExpression::walk(l);

    valueIdMap = std::dynamic_pointer_cast<storage::BaseDictionary<T>>(table->dictionaryAt(field));

    lower_bound.table = 0;
    lower_bound.valueId = valueIdMap->getValueIdForValue(lower_value);
    lower_value_exists = valueIdMap->isValueIdValid(lower_bound.valueId) && lower_value == valueIdMap->getValueForValueId(lower_bound.valueId);
    upper_bound.table = 0;
    upper_bound.valueId = valueIdMap->getValueIdForValue(upper_value);
    upper_value_exists = valueIdMap->isValueIdValid(upper_bound.valueId) && upper_value == valueIdMap->getValueForValueId(upper_bound.valueId);
  }


  virtual ~BetweenExpression() {}

  inline virtual bool operator()(size_t row) {
    ValueId valueId = table->getValueId(field, row);

    if ((valueId.table == lower_bound.table) && (valueId.table == upper_bound.table)) {
      if ((valueId.valueId <= upper_bound.valueId) && (valueId.valueId >= lower_bound.valueId)) {
        return true;
      }

      if ((valueId.valueId > upper_bound.valueId) || (valueId.valueId < lower_bound.valueId)) {
        return false;
      }

      if (upper_value_exists && lower_value_exists) {
        return false;
      }
    }

    T value = table->getValue<T>(field, row);
    return (value <= upper_value) && (value >= lower_value);
  }
};

} } // namespace hyrise::access

