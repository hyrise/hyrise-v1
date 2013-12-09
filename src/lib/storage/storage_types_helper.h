// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>

#include "storage/meta_storage.h"
#include "storage/cast_functor.h"
#include "storage/PointerCalculator.h"

namespace hyrise {

std::string data_type_to_string(DataType d);

pos_list_t *pos_list_intersection(pos_list_t *p1, pos_list_t *p2, const bool deleteInput = false);

struct HyriseHelper {

  /**
   * Templated method for casting a value by column and value-ID into a given type.
   *
   * @param column  Column of the cell containing the value.
   * @param valueId ID of the value to be casted.
   */
  template <typename T>
  static T castValue(const storage::AbstractTable *table, const size_t column, const ValueId valueId) {
    storage::cast_functor_by_value_id<T> f(const_cast<storage::AbstractTable *>(table), column, valueId);
    storage::type_switch<hyrise_basic_types> ts;
    return ts(table->typeOfColumn(column), f);
  }


  /**
   * Templated method for casting a value by column and row into a given type.
   *
   * @param column Column of the cell containing the value.
   * @param row    Row of the cell containing the value.
   */
  template <typename T>
  static T castValueByColumnRow(const storage::AbstractTable *table, const size_t column, const size_t row) {
    storage::cast_functor_by_row<T> f(const_cast<storage::AbstractTable *>(table), column, row);
    storage::type_switch<hyrise_basic_types> ts;
    return ts(table->typeOfColumn(column), f);
  }

};

} // namespace hyrise

