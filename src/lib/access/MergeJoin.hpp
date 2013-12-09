// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MERGEJOIN_HPP_
#define SRC_LIB_ACCESS_MERGEJOIN_HPP_

#include "access/system/PlanOperation.h"

#include "access/expressions/predicates.h"

#include "storage/AbstractTable.h"
#include "storage/MutableVerticalTable.h"
#include "storage/PointerCalculator.h"

namespace hyrise {
namespace access {

template <typename T>
class MergeJoin : public PlanOperation {
public:
  virtual ~MergeJoin() {}

  void executePlanOperation() {
    if (!producesPositions) {
      throw std::runtime_error("MergeJoin execute() not supported with producesPositions == false");
    }

    size_t left_input_size = input.getTable(0)->size();
    std::vector<std::pair<T, pos_t> > left_values;

    T value;

    for (pos_t row = 0; row < left_input_size; row++) {
      const auto &t = input.getTable(0);
      value = t->template getValue<T>(_field_definition[0], row);
      left_values.push_back(std::pair<T, storage::pos_t>(value, row));
    }

    std::sort(left_values.begin(), left_values.end());

    size_t right_input_size = input.getTable(1)->size();
    std::vector<std::pair<T, storage::pos_t> > right_values;

    for (pos_t row = 0; row < right_input_size; row++) {
      const auto& t = input.getTable(1);
      value = t->template getValue<T>(_field_definition[1], row);
      right_values.push_back(std::pair<T, storage::pos_t>(value, row));
    }

    std::sort(right_values.begin(), right_values.end());

    std::vector<storage::pos_t> *left_pos = new std::vector<storage::pos_t>();
    std::vector<storage::pos_t> *right_pos = new std::vector<storage::pos_t>();

    pos_t left_i = 0, right_i = 0;
    std::pair<T, storage::pos_t> left_value = left_values[0];
    std::pair<T, storage::pos_t> right_value = right_values[0];

    while (left_i < left_input_size && right_i < right_input_size) {
      if (right_value.first == left_value.first) {
        left_pos->push_back(left_value.second);
        right_pos->push_back(right_value.second);

        if (left_i + 1 < left_input_size && left_values[left_i + 1].first == left_value.first) {
          left_i++;
          left_value = left_values[left_i];
        } else {
          right_i++;

          if (right_i < right_input_size) {
            right_value = right_values[right_i];
          }
        }
      } else if (right_value.first < left_value.first) {
        right_i++;

        if (right_i < right_input_size) {
          right_value = right_values[right_i];
        }

      } else {
        left_i++;

        if (left_i < left_input_size) {
          left_value = left_values[left_i];
        }
      }
    }

    std::vector<storage::atable_ptr_t> parts({
      std::dynamic_pointer_cast<storage::AbstractTable>(storage::PointerCalculator::create(input.getTable(0), left_pos)),
      std::dynamic_pointer_cast<storage::AbstractTable>(storage::PointerCalculator::create(input.getTable(1), right_pos))
    });

    addResult(std::make_shared<storage::MutableVerticalTable>(parts));
  }

  const std::string vname() {
    return "MergeJoin";
  }
};

}
}

#endif  // SRC_LIB_ACCESS_MERGEJOIN_HPP_
