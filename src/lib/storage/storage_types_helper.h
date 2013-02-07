// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_STORAGE_TYPES_HELPER_H_
#define SRC_LIB_STORAGE_STORAGE_TYPES_HELPER_H_

#include <memory>

#include "storage/meta_storage.h"
#include "storage/cast_functor.h"
#include "storage/PointerCalculator.h"

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
  static T castValue(const AbstractTable *table, const size_t column, const ValueId valueId) {
    hyrise::storage::cast_functor_by_value_id<T> f(const_cast<AbstractTable *>(table), column, valueId);
    hyrise::storage::type_switch<hyrise_basic_types> ts;
    return ts(table->typeOfColumn(column), f);
  }


  /**
   * Templated method for casting a value by column and row into a given type.
   *
   * @param column Column of the cell containing the value.
   * @param row    Row of the cell containing the value.
   */
  template <typename T>
  static T castValueByColumnRow(const AbstractTable *table, const size_t column, const size_t row) {
    hyrise::storage::cast_functor_by_row<T> f(const_cast<AbstractTable *>(table), column, row);
    hyrise::storage::type_switch<hyrise_basic_types> ts;
    return ts(table->typeOfColumn(column), f);
  }

};



#endif  // SRC_LIB_STORAGE_STORAGE_TYPES_HELPER_H_

