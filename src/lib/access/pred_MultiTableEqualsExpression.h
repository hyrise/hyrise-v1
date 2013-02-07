// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PRED_MULTITABLEEQUALSEXPRESSION_H_
#define SRC_LIB_ACCESS_PRED_MULTITABLEEQUALSEXPRESSION_H_

#include "pred_common.h"

// Simple Template for equals expression with multiple tables
template <typename T>
class MultiTableEqualsExpression : public SimpleFieldExpression {
 protected:

  std::vector<ValueId> ids;
  std::vector<bool> has_vid;
  T value;
  size_t input;

 public:

  MultiTableEqualsExpression(size_t _input, field_t _field, T _value):
      SimpleFieldExpression(_input, _field), value(_value)
  {}

  MultiTableEqualsExpression(size_t _input, field_name_t _field, T _value):
      SimpleFieldExpression(_input, _field), value(_value)
  {}

  MultiTableEqualsExpression(hyrise::storage::c_store_ptr_t _table, field_t _field, T _value) :
      SimpleFieldExpression(std::dynamic_pointer_cast<const AbstractTable>(_table), _field), value(_value)
  {}

  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t > &l) {
    SimpleFieldExpression::walk(l);

    // According to the number of tables
    size_t subtables = table->subtableCount();

    // Reserve the values for the value id and for the
    // bool list
    ValueId vid;
    ids.assign(subtables, vid);
    has_vid.assign(subtables, false);


    for (table_id_t i = 0; i < subtables; ++i) {
      if (table->valueExists(field, value, i)) {
        ids[i] = table->getValueIdForValueByTableId(field, value, false, i);
        has_vid[i] = true;
      }
    }
  }

  virtual ~MultiTableEqualsExpression() { }

  inline virtual bool operator()(size_t row) {
    ValueId v = table->getValueId(field, row);

    if (has_vid[v.table] && ids[v.table].valueId == v.valueId) {
      return true;
    }

    return false;
  }
};
#endif  // SRC_LIB_ACCESS_PRED_MULTITABLEEQUALSEXPRESSION_H_
