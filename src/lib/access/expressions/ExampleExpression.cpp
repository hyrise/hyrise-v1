// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/expressions/ExampleExpression.h"

#include "storage/AbstractTable.h"

#include "helper/make_unique.h"
#include "access/expressions/ExpressionRegistration.h"

namespace hyrise { namespace access {

namespace {
auto _ = Expressions::add<ExampleExpression>("hyrise::example");
}

ExampleExpression::ExampleExpression(const size_t& column, const hyrise_int_t& value) : _column(column), _value(value) {}

inline bool ExampleExpression::operator()(const size_t& row) {  return _vector->getRef(_column, row) == _valueid;  }

pos_list_t* ExampleExpression::match(const size_t start, const size_t stop) {
  auto pl = new pos_list_t;
  for(size_t row=start; row < stop; ++row) {
    if (this->ExampleExpression::operator()(row)) {
      pl->push_back(row);
    }
  }
  return pl;
}

void ExampleExpression::walk(const std::vector<storage::c_atable_ptr_t> &tables) {
  _table = tables.at(0);
  const auto& avs = _table->getAttributeVectors(_column);
  _vector = std::dynamic_pointer_cast<storage::FixedLengthVector<value_id_t>>(avs.at(0).attribute_vector);
  _dict = std::dynamic_pointer_cast<storage::OrderPreservingDictionary<hyrise_int_t>>(_table->dictionaryAt(_column));
  if (!(_vector && _dict)) throw std::runtime_error("Could not extract proper structures");
  _valueid = _dict->getValueIdForValue(_value);
}
    
std::unique_ptr<ExampleExpression> ExampleExpression::parse(const Json::Value& data) {
  return make_unique<ExampleExpression>(data["column"].asUInt(), data["value"].asUInt());
}

}}
