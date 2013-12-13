// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "helper/types.h"
#include "pred_common.h"

namespace hyrise {
namespace access {

class SimpleFieldExpression : public SimpleExpression {
 protected:
  storage::c_atable_ptr_t table;
  field_t field;
  field_name_t field_name;
  size_t input;
 public:

  SimpleFieldExpression(size_t input_index, field_t field_index): field(field_index),
                                                                  input(input_index) { }

  SimpleFieldExpression(storage::c_atable_ptr_t table, field_t field_index) : table(table),
                                                                                    field(field_index),
                                                                                    input(0) { }

  SimpleFieldExpression(size_t input_index, field_name_t field_name): field(0),
                                                                      field_name(field_name),
                                                                      input(input_index) { }

  SimpleFieldExpression(storage::c_atable_ptr_t table, field_name_t field_name) : table(table),
                                                                                        field(0),
                                                                                        field_name(field_name),
                                                                                        input(0) { }


  virtual ~SimpleFieldExpression() { }

  virtual void walk(const std::vector<storage::c_atable_ptr_t > &l) {
    if (!table) {
      table = l.at(input);
    }

    if ((field == 0) && (field_name.size() > 0)) {
      field = table->numberOfColumn(field_name);
    }
  }

  inline virtual bool operator()(size_t row) {
    throw std::runtime_error("Cannot call base class");
  }
};

template <typename T, class Op = std::equal_to<T> >
class GenericExpressionValue : public SimpleFieldExpression {
 private:
  T value;
  Op _operator;

 public:

  GenericExpressionValue(size_t i, field_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  GenericExpressionValue(size_t i, field_name_t f, T _value):
      SimpleFieldExpression(i, f), value(_value)
  {}

  GenericExpressionValue(const storage::c_atable_ptr_t& _table, field_t _field, T _value) :
      SimpleFieldExpression(_table, _field), value(_value)
  {}


  virtual ~GenericExpressionValue() { }

  inline virtual bool operator()(size_t row) {
    return _operator(table->template getValue<T>(field, row), value);
  }
};

} } // namespace hyrise::access

