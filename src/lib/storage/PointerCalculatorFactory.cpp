
#include "PointerCalculator.h"
#include "MutableVerticalTable.h"
#include "PointerCalculatorFactory.h"

std::shared_ptr<PointerCalculator> PointerCalculatorFactory::createPointerCalculator(hyrise::storage::c_atable_ptr_t _table, std::vector<field_t> *_field_definition, std::vector<pos_t> *_positions) {
  return std::make_shared<PointerCalculator>(_table, _positions, _field_definition);
}

std::shared_ptr<PointerCalculator> PointerCalculatorFactory::createPointerCalculatorNonRef(hyrise::storage::c_atable_ptr_t _table, std::vector<field_t> *_field_definition, std::vector<pos_t> *_positions) {
  return std::make_shared<PointerCalculator>(_table, _positions, _field_definition);
}

std::shared_ptr<PointerCalculator> PointerCalculatorFactory::createView(hyrise::storage::c_atable_ptr_t& _table, size_t start, size_t end) {
  pos_list_t *positions = new pos_list_t;
  for (pos_t row = start; row <= end; ++row)
    positions->push_back(row);

  return std::make_shared<PointerCalculator>(_table, positions);
}
