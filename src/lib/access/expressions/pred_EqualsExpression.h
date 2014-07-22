// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "helper/types.h"
#include "helper/checked_cast.h"
#include "pred_common.h"

// Required for Raw Table Scan
#include <storage/RawTable.h>
#include "storage/column_extract.h"

namespace hyrise {
namespace access {

template <typename T>
class EqualsExpression : public SimpleFieldExpression {
 private:
  ValueId lower_bound;
  std::shared_ptr<storage::BaseDictionary<T>> valueIdMap;
  bool value_exists;

 public:
  T value;

  EqualsExpression(size_t _input, field_t _field, T _value) : SimpleFieldExpression(_input, _field), value(_value) {}

  EqualsExpression(size_t _input, field_name_t _field, T _value)
      : SimpleFieldExpression(_input, _field), value(_value) {}


  EqualsExpression(storage::c_atable_ptr_t& _table, field_t _field, T _value)
      : SimpleFieldExpression(_table, _field), value(_value) {}

  virtual void walk(const std::vector<storage::c_atable_ptr_t>& l) {
    SimpleFieldExpression::walk(l);
    valueIdMap = std::dynamic_pointer_cast<storage::BaseDictionary<T>>(table->dictionaryAt(field));

    value_exists =
        (lower_bound.valueId = valueIdMap->findValueIdForValue(value)) != std::numeric_limits<value_id_t>::max();
  }

  virtual std::unique_ptr<AbstractExpression> clone() {
    return std::unique_ptr<EqualsExpression<T>>(new EqualsExpression<T>(table, field, value));
  }

  virtual ~EqualsExpression() {}

  inline virtual bool operator()(size_t row) { return value_exists && table->getValue<T>(field, row) == value; }

  virtual storage::pos_list_t matchAll(storage::pos_list_t& pos) {
      storage::pos_list_t res;
      auto it = pos.begin(), e = pos.end();
      while (it != e) {

          it++;
      }
      return res;
  }
  virtual storage::pos_list_t matchAll(size_t start, size_t stop) {
      auto parts = storage::column_parts_extract(*table.get(), field);
      storage::pos_list_t results;
      for (auto part : parts) {
          if (part.verticalStart > start) break;
          if (part.verticalEnd > start) continue; // next part, not yet at the right part

          auto dict = checked_pointer_cast<storage::BaseDictionary<T>>(part.table.dictionaryAt(part.columnOffset));
          if (dict->valueExists(value)) continue; // next part, this one doesn't have our value
          auto vid = dict->getValueIdForValue(value);
          auto aav = checked_pointer_cast<storage::BaseAttributeVector<value_id_t> >(part.table.getAttributeVectors(part.columnOffset).at(0).attribute_vector);

          for (std::size_t p = start - part.verticalStart, e = p + part.verticalEnd - stop; p < e; p++) {
              if (aav->get(part.columnOffset, p) == vid) results.push_back(part.verticalStart + p);
          }
      }
      return results;

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

  EqualsExpressionRaw(size_t _input, field_t _field, T _value) : SimpleFieldExpression(_input, _field), value(_value) {}

  EqualsExpressionRaw(size_t _input, field_name_t _field, T _value)
      : SimpleFieldExpression(_input, _field), value(_value) {}

  EqualsExpressionRaw(const storage::c_atable_ptr_t& _table, field_t _field, T _value)
      : SimpleFieldExpression(_table, _field), value(_value) {}

  virtual ~EqualsExpressionRaw() {}

  inline virtual bool operator()(size_t row) {
    return (std::dynamic_pointer_cast<const storage::RawTable>(table))->template getValue<T>(field, row) == value;
  }
};
}
}  // namespace hyrise::access
