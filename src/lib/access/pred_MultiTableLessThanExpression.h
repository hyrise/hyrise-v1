// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PRED_MULTITABLELESSTHANEXPRESSION_H_
#define SRC_LIB_ACCESS_PRED_MULTITABLELESSTHANEXPRESSION_H_

#include "pred_common.h"

template <typename T>
class MultiTableLessThanExpression: public SimpleFieldExpression {
 protected:
  std::vector<ValueId> ids;
  std::vector<bool> has_vid;
  T value;

  std::vector<bool> _sorteds;

 public:
  MultiTableLessThanExpression(size_t i, field_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  MultiTableLessThanExpression(size_t i, field_name_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  MultiTableLessThanExpression(hyrise::storage::c_store_ptr_t _table, field_t _field, T _value):
      SimpleFieldExpression(std::dynamic_pointer_cast<const AbstractTable>(_table), _field), value(_value)
  {}

  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t > &l) {
    SimpleFieldExpression::walk(l);

    size_t subtables = table->subtableCount();
    _sorteds.assign(subtables, true);

    // Reserve the values for the value id and for the
    // bool list
    ValueId vid;
    ids.assign(subtables, vid);
    has_vid.assign(subtables, false);


    for (table_id_t i = 0; i < subtables; ++i) {
      _sorteds[i] = table->dictionaryByTableId(field, i)->isOrdered();

      if (table->valueExists(field, value, i)) {
        ids[i] = table->getValueIdForValueByTableId(field, value, false, i);
        has_vid[i] = true;

      } else if (_sorteds[i]) {
        // If the value does not exist we have to find the first value smaller
        ValueId v2;
        v2.valueId = std::dynamic_pointer_cast<BaseDictionary<T>>(table->dictionaryByTableId(field, i))->getValueIdForValueSmaller(value);
        v2.table = i;

        ids[i] = v2;
      }
    }
  }


  inline virtual bool operator()(size_t row) {
    ValueId v = table->getValueId(this->field, row);

    if (!_sorteds[v.table]) {
      T tt = table->getValueForValueId<T>(this->field, v);
      return tt < this->value;
    }

    if (this->has_vid[v.table] &&  v.valueId < this->ids[v.table].valueId) {
      return true;

    } else if (!this->has_vid[v.table] &&  v.valueId <= this->ids[v.table].valueId) {
      return true;
    }

    return false;
  }
};
#endif  // SRC_LIB_ACCESS_PRED_MULTITABLELESSTHANEXPRESSION_H_
