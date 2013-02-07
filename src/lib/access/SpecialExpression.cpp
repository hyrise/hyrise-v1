// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SpecialExpression.h"

#include "storage/AbstractTable.h"

namespace hyrise { namespace access {

SpecialExpression::SpecialExpression(const size_t& column, const hyrise_int_t& value) : _column(column), _value(value) {}

inline bool SpecialExpression::operator()(const size_t& row) {  return _vector->getRef(_column, row) == _valueid;  }

pos_list_t* SpecialExpression::match(const size_t start, const size_t stop) {
  auto pl = new pos_list_t;
  for(size_t row=0; row < stop; ++row) {
    if (this->SpecialExpression::operator()(row)) {
      pl->push_back(row);
    }
  }
  return pl;
}

void SpecialExpression::walk(const std::vector<hyrise::storage::c_atable_ptr_t> &tables) {
  _table = tables.at(0);
  const auto& avs = _table->getAttributeVectors(_column);
  _vector = std::dynamic_pointer_cast<FixedLengthVector<value_id_t>>(avs.at(0).attribute_vector);
  _dict = std::dynamic_pointer_cast<OrderPreservingDictionary<hyrise_int_t>>(_table->dictionaryAt(_column));
  if (!(_vector && _dict)) throw std::runtime_error("Could not extract proper structures");
  _valueid = _dict->getValueIdForValue(_value);
}

}}
