// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PRED_MULTITABLEBETWEENEXPRESSION_H_
#define SRC_LIB_ACCESS_PRED_MULTITABLEBETWEENEXPRESSION_H_

#include "pred_common.h"

template <typename T>
class MultiTableBetweenExpression: public SimpleFieldExpression {

 protected:

  T lower_value;
  T upper_value;

  std::vector<ValueId> lids;
  std::vector<ValueId> uids;
  std::vector<bool> has_lvid;
  std::vector<bool> has_uvid;
  std::vector<bool> _sorteds;

 public:

  MultiTableBetweenExpression(size_t i, field_t _field, T _lower_value, T _upper_value):
      SimpleFieldExpression(i, _field), lower_value(_lower_value), upper_value(_upper_value)
  {}

  MultiTableBetweenExpression(size_t i, field_name_t _field, T _lower_value, T _upper_value):
      SimpleFieldExpression(i, _field), lower_value(_lower_value), upper_value(_upper_value)
  {}

  MultiTableBetweenExpression(hyrise::storage::c_store_ptr_t _table, field_t _field, T _lower_value, T _upper_value):
      SimpleFieldExpression(std::dynamic_pointer_cast<const AbstractTable>(_table), _field), lower_value(_lower_value), upper_value(_upper_value)
  {}

  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t > &l) {
    SimpleFieldExpression::walk(l);

    size_t subtables = table->subtableCount();
    _sorteds.assign(subtables, true);

    // Reserve the values for the value id and for the
    // bool list
    ValueId vid;
    lids.assign(subtables, vid);
    uids.assign(subtables, vid);
    has_lvid.assign(subtables, false);
    has_uvid.assign(subtables, false);

    for (table_id_t i = 0; i < subtables; ++i) {
      _sorteds[i] = table->dictionaryByTableId(field, i)->isOrdered();

      if (table->valueExists(field, lower_value, i)) {
        lids[i] = table->getValueIdForValueByTableId(field, lower_value, false, i);
        has_lvid[i] = true;

      } else if (_sorteds[i]) {
        // If the value does not exist we have to find the first value smaller
        ValueId v2;
        v2.valueId = std::dynamic_pointer_cast<BaseDictionary<T>>(table->dictionaryByTableId(field, i))->getValueIdForValueSmaller(lower_value);
        v2.table = i;

        lids[i] = v2;
      }

      if (table->valueExists(field, upper_value, i)) {
        uids[i] = table->getValueIdForValueByTableId(field, upper_value, false, i);
        has_uvid[i] = true;

      } else if (_sorteds[i]) {
        // If the value does not exist we have to find the first value smaller
        ValueId v2;
        v2.valueId = std::dynamic_pointer_cast<BaseDictionary<T>>(table->dictionaryByTableId(field, i))->getValueIdForValueSmaller(upper_value);
        v2.table = i;

        uids[i] = v2;
      }
    }
  }

  inline virtual bool operator()(size_t row) {
    ValueId v = table->getValueId(this->field, row);

    if (!_sorteds[v.table]) {
      T tt = table->getValueForValueId<T>(this->field, v);
      return tt > this->lower_value && tt < this->upper_value;
    }

    if (this->has_lvid[v.table] && this->has_uvid[v.table] && v.valueId > this->lids[v.table].valueId && v.valueId < this->uids[v.table].valueId) {
      return true;

    } else if (!this->has_lvid[v.table] && !this->has_uvid[v.table] && v.valueId >= this->lids[v.table].valueId && v.valueId <= this->uids[v.table].valueId) {
      return true;
    }

    return false;
  }
};
#endif  // SRC_LIB_ACCESS_PRED_MULTITABLEBETWEENEXPRESSION_H_
