// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PRED_EQUALSEXPRESSION_H_
#define SRC_LIB_ACCESS_PRED_EQUALSEXPRESSION_H_

#include "helper/types.h"

#include "pred_common.h"

// Required for Raw Table Scan
#include <storage/RawTable.h>

template <typename T>
class EqualsExpression : public SimpleFieldExpression {
 private:
  ValueId lower_bound;
  std::shared_ptr<BaseDictionary<T>> valueIdMap;
  bool value_exists;

 public:

  T value;

  EqualsExpression(size_t _input, field_t _field, T _value):
      SimpleFieldExpression(_input, _field), value(_value)
  {}

  EqualsExpression(size_t _input, field_name_t _field, T _value):
      SimpleFieldExpression(_input, _field), value(_value)
  {}


  EqualsExpression(hyrise::storage::c_atable_ptr_t& _table, field_t _field, T _value) : SimpleFieldExpression(_table, _field), value(_value)
  {}

  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t > &l) {
    SimpleFieldExpression::walk(l);
    valueIdMap = std::dynamic_pointer_cast<BaseDictionary<T>>(table->dictionaryAt(field));

    value_exists = valueIdMap->valueExists(value);

    if (value_exists) {
      lower_bound = table->getValueIdForValue(field, value);
    }
  }

  virtual ~EqualsExpression() { }

  inline virtual bool operator()(size_t row) {
    return value_exists && table->getValueId(field, row) == lower_bound;
  }
};

/**
 * Equals Expressions that uses directly the raw value instead of
 * working with value ids of the dictionary encoded column.
 */
template <typename T>
class EqualsExpressionRaw : public SimpleFieldExpression {

 public:

  T value;

  EqualsExpressionRaw(size_t _input, field_t _field, T _value):
      SimpleFieldExpression(_input, _field), value(_value)
  {}

  EqualsExpressionRaw(size_t _input, field_name_t _field, T _value):
      SimpleFieldExpression(_input, _field), value(_value)
  {}

  EqualsExpressionRaw(const hyrise::storage::c_atable_ptr_t& _table, field_t _field, T _value) : SimpleFieldExpression(_table, _field), value(_value)
  {}

  virtual ~EqualsExpressionRaw() { }

  inline virtual bool operator()(size_t row) {
    return (std::dynamic_pointer_cast<const RawTable>(table))->template getValue<T>(field, row) == value;
  }
};

#endif  // SRC_LIB_ACCESS_PRED_EQUALSEXPRESSION_H_

