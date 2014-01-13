// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/UpdateScan.h"

#include "storage/Store.h"

namespace hyrise {
namespace access {

UpdateScan::UpdateScan() {
  _comparator = nullptr;
  _func = nullptr;
}

UpdateScan::~UpdateScan() {
}

void UpdateScan::executePlanOperation() {
  size_t input_size = input.getTable(0)->size();
  std::map<int, int> mapping;

  if (_data) {
    size_t data_size = _data->size();

    if (data_size != 1) {
      throw std::runtime_error("Too many rows in update data");
    }

    for (size_t i = 0; i < _data->columnCount(); i++) {
      auto src_field = _data->metadataAt(i);

      for (size_t j = 0; i < input.getTable(0)->columnCount(); j++) {
        auto tgt_field = input.getTable(0)->metadataAt(j);

        if (tgt_field.matches(src_field)) {
          mapping[i] = j;
          break;
        }
      }

      if (mapping.count(i) != 1) {
        throw std::runtime_error(src_field.getName() + " did not find a match!");
      }
    }
  }

  std::map<int, int>::iterator it;

  auto s = std::dynamic_pointer_cast<const storage::Store>(input.getTable(0));

  if (!s) {
    throw std::runtime_error("Updates not supported for non delta structures");
  }

  size_t delta_row = s->getDeltaTable()->size();

  for (size_t row = 0; row < input_size; ++row) {
    // Execute the predicate on the list
    if ((*_comparator)(row)) {
      if (_func != nullptr) {
        _func->updateRow(row);
      } else {
        for (it = mapping.begin(); it != mapping.end(); it++) {
          int src = (*it).first;
          int tgt = (*it).second;

          // Update the delta
          s->getDeltaTable()->copyRowFrom(s, row, delta_row, true);
          s->getDeltaTable()->copyValueFrom(_data, src, 0, tgt, delta_row++);
        }
      }
    }
  }

  addResult(input.getTable(0));
}

const std::string UpdateScan::vname() {
  return "UpdateScan";
}

void UpdateScan::setUpdateTable(const storage::atable_ptr_t &c) {
  _data = c;
}

void UpdateScan::setUpdateFunction(UpdateFun *f) {
  _func = f;
}

void UpdateScan::setPredicate(SimpleExpression *e) {
  _comparator = e;
}

}
}
