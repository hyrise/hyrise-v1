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
  BetweenExpression(size_t i, field_t f, T _lower_value, T _upper_value)
      : SimpleFieldExpression(i, f), lower_value(_lower_value), upper_value(_upper_value) {
    if (lower_value > upper_value)
      std::swap(lower_value, upper_value);
  }

  BetweenExpression(size_t i, field_name_t f, T _lower_value, T _upper_value)
      : SimpleFieldExpression(i, f), lower_value(_lower_value), upper_value(_upper_value) {
    if (lower_value > upper_value)
      std::swap(lower_value, upper_value);
  }

  BetweenExpression(storage::c_atable_ptr_t _table, field_t _field, T _lower_value, T _upper_value)
      : SimpleFieldExpression(_table, _field), lower_value(_lower_value), upper_value(_upper_value) {}

  virtual void walk(const std::vector<storage::c_atable_ptr_t>& l) {
    SimpleFieldExpression::walk(l);

    valueIdMap = std::dynamic_pointer_cast<storage::BaseDictionary<T>>(table->dictionaryAt(field));

    lower_bound.valueId = valueIdMap->getValueIdForValue(lower_value);
    lower_value_exists = valueIdMap->isValueIdValid(lower_bound.valueId) &&
                         lower_value == valueIdMap->getValueForValueId(lower_bound.valueId);
    upper_bound.valueId = valueIdMap->getValueIdForValue(upper_value);
    upper_value_exists = valueIdMap->isValueIdValid(upper_bound.valueId) &&
                         upper_value == valueIdMap->getValueForValueId(upper_bound.valueId);
    if (!upper_value_exists || !lower_value_exists)
      throw std::logic_error("Between only works for values with existing value ids");
  }


  virtual ~BetweenExpression() {}

  inline virtual bool operator()(size_t row) {
    ValueId valueId = table->getValueId(field, row);
    return (lower_bound.valueId <= valueId.valueId) && (valueId.valueId <= upper_bound.valueId);
  }
};
}
}  // namespace hyrise::access
