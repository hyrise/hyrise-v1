// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MERGEJOIN_HPP_
#define SRC_LIB_ACCESS_MERGEJOIN_HPP_

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <access/PlanOperation.h>
#include <access/predicates.h>
#include <storage/PointerCalculator.h>
#include <storage/PointerCalculatorFactory.h>

template <typename T>
class MergeJoin : public _PlanOperation {
 public:

  MergeJoin() {
    
  }

  virtual ~MergeJoin() {
    
  }

  /*
   * Expects two inputs
   * @return in the return value, returns the second input and the resulting positions
   */
  void executePlanOperation() {
    if (!producesPositions) {
      throw std::runtime_error("MergeJoin execute() not supported with generatesPositions == false");
    }

    size_t left_input_size = input.getTable(0)->size();
    std::vector<std::pair<T, pos_t> > left_values;

    T value;

    for (pos_t row = 0; row < left_input_size; row++) {
      const auto& t = input.getTable(0);
      value = t->getValue<T>(_field_definition[0], row);
      left_values.push_back(std::pair<T, pos_t>(value, row));
    }

    std::sort(left_values.begin(), left_values.end());

    size_t right_input_size = input.getTable(1)->size();
    std::vector<std::pair<T, pos_t> > right_values;

    for (pos_t row = 0; row < right_input_size; row++) {
      const auto& t = input.getTable(1);
      value = t->getValue<T>(_field_definition[1], row);
      right_values.push_back(std::pair<T, pos_t>(value, row));
    }

    std::sort(right_values.begin(), right_values.end());

    std::vector<pos_t> *left_pos = new std::vector<pos_t>();
    std::vector<pos_t> *right_pos = new std::vector<pos_t>();

    pos_t left_i = 0, right_i = 0;
    std::pair<T, pos_t> left_value = left_values[0];
    std::pair<T, pos_t> right_value = right_values[0];

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

    std::vector<hyrise::storage::c_atable_ptr_t> parts({
      std::dynamic_pointer_cast<const AbstractTable>(PointerCalculatorFactory::createPointerCalculatorNonRef(input.getTable(0), nullptr, left_pos)),
      std::dynamic_pointer_cast<const AbstractTable>(PointerCalculatorFactory::createPointerCalculatorNonRef(input.getTable(1), nullptr, right_pos))
    });
    addResult(std::make_shared<const MutableVerticalTable>(parts));
  }

  const std::string vname() {
    return "MergeJoin";
  }

};
#endif  // SRC_LIB_ACCESS_MERGEJOIN_HPP_

