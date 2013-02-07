// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_POINTERCALCULATORFACTORY_H_
#define SRC_LIB_STORAGE_POINTERCALCULATORFACTORY_H_

#include <memory>

#include "helper/types.h"

#include "storage/storage_types.h"

class AbstractTable;
class PointerCalculator;

class PointerCalculatorFactory {
 public:

  PointerCalculatorFactory() {};

  static std::shared_ptr<PointerCalculator> createPointerCalculator(hyrise::storage::c_atable_ptr_t _table, field_list_t *_field_definition = nullptr, pos_list_t *_positions = NULL);

  static std::shared_ptr<PointerCalculator> createPointerCalculatorNonRef(hyrise::storage::c_atable_ptr_t _table, field_list_t *_field_definition = nullptr, pos_list_t *_positions = NULL);

  static std::shared_ptr<PointerCalculator> createView(hyrise::storage::c_atable_ptr_t& _table, size_t start, size_t end);

};

#endif  // SRC_LIB_STORAGE_POINTERCALCULATORFACTORY_H_
