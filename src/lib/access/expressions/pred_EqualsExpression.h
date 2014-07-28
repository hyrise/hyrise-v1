// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <algorithm>

#include "helper/types.h"
#include "helper/checked_cast.h"
#include "storage/column_extract.h"
#include "pred_common.h"

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

  inline void op(const storage::BaseAttributeVector<value_id_t>& aav,
                 size_t col,
                 size_t av_pos,
                 size_t abs_pos,
                 value_id_t vid,
                 storage::pos_list_t& results) {
    if (aav.get(col, av_pos) == vid) {
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
    for (const auto& part : parts) {
      //  std::cout << part << std::endl;
      if (part.iterStart == part.iterEnd)
        continue;
      auto dict = checked_pointer_cast<storage::BaseDictionary<T>>(part.table.dictionaryAt(part.columnOffset));
      if (!dict->valueExists(value)) {
        while (*(it++) < part.verticalEnd) {
        }
        continue;
      }

      auto vid = dict->getValueIdForValue(value);
      auto aav = checked_pointer_cast<storage::BaseAttributeVector<value_id_t>>(
          part.table.getAttributeVectors(part.columnOffset).at(0).attribute_vector);
      do {
        // std::cout << "checking" << *it << " " << pos.size() << std::endl;
        auto p = *it - part.verticalStart;

        op(*aav.get(), part.columnOffset, p, part.verticalStart + p, vid, results);
        // if (aav->get(part.columnOffset, p) == vid) {
        // results.push_back(part.verticalStart + p);
        //}
        ++it;
      } while ((*it < part.verticalEnd) && (it != et));
    }
    // std::cout << results.size() << std::endl;
    return results;
  }

  virtual storage::pos_list_t matchAll(size_t start, size_t stop) {
    auto parts = storage::column_parts_extract(*table.get(), field, start, stop);
    assert(parts.size() != 0);
    storage::pos_list_t results;

    for (const auto& part : parts) {
      // std::cout << part << std::endl;
      if (part.iterStart == part.iterEnd)
        continue;
      auto pdict = part.table.dictionaryAt(part.columnOffset);

      auto dict = checked_pointer_cast<storage::BaseDictionary<T>>(pdict);
      if (!dict->valueExists(value)) {
        // std::cout << "Bailing, vid not found" << std::endl;
        continue;  // next part, this one doesn't have our value
      }
      auto vid = dict->getValueIdForValue(value);
      auto aav = checked_pointer_cast<storage::BaseAttributeVector<value_id_t>>(
          part.table.getAttributeVectors(part.columnOffset).at(0).attribute_vector);

      for (std::size_t p = part.iterStart, e = part.iterEnd; p < e; p++) {
        // std::cout << "checking" << p << " " << e << std::endl;
        op(*aav.get(), part.columnOffset, p, part.verticalStart + p, vid, results);
        // if (aav->get(part.columnOffset, p) == vid) {
        // results.push_back(part.verticalStart + p);
        //}
      }
      start = part.verticalEnd + 1;
    }
    // std::cout << results.size() << std::endl;
    return results;
  }
};
}
}  // namespace hyrise::access
