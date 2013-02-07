// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INSERTONLY_VALID_H_
#define SRC_LIB_ACCESS_INSERTONLY_VALID_H_

#include "helper/types.h"
#include "storage/PointerCalculatorFactory.h"
#include "storage/PointerCalculator.h"

namespace hyrise { namespace insertonly {

/// Computation of valid positions
/// @input[in] store store to extract valid positions from
/// @input[in] tid transaction id at which validity is computed
/// @returns position list
template<typename T>
storage::pos_list_t validPositions(const T& store, const tx::transaction_id_t& tid) {
  storage::pos_list_t positions;
  const auto& valid_to_position = store->numberOfColumn(VALID_TO_COL_ID);
  const auto& valid_from_position = store->numberOfColumn(VALID_FROM_COL_ID);
  for (size_t row=0, rows=store->size(); row < rows; ++row) {
    const auto& toValue = store->template getValue<hyrise_int_t>(valid_to_position, row);
    if (((toValue == VISIBLE) || (toValue > tid)) &&
        (store->template getValue<hyrise_int_t>(valid_from_position, row) <= tid)) {
      positions.push_back(row);
    }
  }
  return positions;
}

/// Computation of valid position calculator on store
/// @input[in] store store to extract valid positions from
/// @input[in] tid transaction id at which validity is computed
/// @returns Position Calculator that wraps positions
template<typename T>
storage::atable_ptr_t validPointerCalculator(const T& store, const tx::transaction_id_t& tid) {
  storage::pos_list_t* positions = new storage::pos_list_t(validPositions(store, tid));
  return PointerCalculatorFactory::createPointerCalculatorNonRef(store, nullptr, positions);
}

}}
#endif
