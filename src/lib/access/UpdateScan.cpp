// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <access/UpdateScan.h>
#include <storage/storage_types.h>
#include <storage/Store.h>

UpdateScan::~UpdateScan() {

}

void UpdateScan::executePlanOperation() {
  // Iterate over the data
  assert(comparator != nullptr);
  assert(data || func != nullptr);

  size_t input_size = input.getTable(0)->size();
  std::map<int, int> mapping;

  if (data) {
    size_t data_size = data->size();

    if (data_size != 1) {
      throw std::runtime_error("Too many rows in update data");
    }

    for (size_t i = 0; i < data->columnCount(); i++) {
      auto src_field = data->metadataAt(i);

      for (size_t j = 0; i < input.getTable(0)->columnCount(); j++) {
        auto tgt_field = input.getTable(0)->metadataAt(j);

        if (tgt_field->matches(src_field)) {
          mapping[i] = j;
          break;
        }
      }

      if (mapping.count(i) != 1) {
        throw std::runtime_error(src_field->getName() + " did not find a match!");
      }
    }
  }

  std::map<int, int>::iterator it;

  auto s = std::dynamic_pointer_cast<const Store>(input.getTable(0));

  if (!s) {
    throw std::runtime_error("Updates not supported for non delta structures");
  }

  size_t delta_row = s->getDeltaTable()->size();

  for (size_t row = 0; row < input_size; ++row) {
    // Execute the predicate on the list
    if ((*comparator)(row)) {

      if (func != nullptr) {
        func->updateRow(row);
      } else {
        for (it = mapping.begin(); it != mapping.end(); it++) {
          int src = (*it).first;
          int tgt = (*it).second;

          // Update the delta
          s->getDeltaTable()->copyRowFrom(s, row, delta_row, true);
          s->getDeltaTable()->copyValueFrom(data, src, 0, tgt, delta_row++);
        }
      }
    }
  }

  // Since we hand the input table through we have to retain the
  // table, because otherwise our internal retain count would be
  // negative
  addResult(input.getTable(0));
}
