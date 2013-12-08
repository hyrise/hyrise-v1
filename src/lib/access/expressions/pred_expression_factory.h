// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "expression_types.h"
#include "../json_converters.h"
#include "predicates.h"

#define GENERATE_EXPRESSION(EXPRESSION)  case PredicateType::EXPRESSION: \
  if (_field_name.size() > 0)                                           \
    return new EXPRESSION<ValueType>(_input_index, _field_name, json_converter::convert<ValueType>(_value)); \
  else                                                                  \
    return new EXPRESSION<ValueType>(_input_index, _field, json_converter::convert<ValueType>(_value)); \
  break;

// for expressions that support only a specific value type
#define GENERATE_EXPRESSION_OF_TYPE(EXPRESSION, TYPE)  case PredicateType::EXPRESSION: \
  if (_field_name.size() > 0)                                           \
    return new EXPRESSION(_input_index, _field_name, json_converter::convert<TYPE>(_value)); \
  else                                                                  \
    return new EXPRESSION(_input_index, _field, json_converter::convert<TYPE>(_value)); \
  break;

#define GENERATE_EXPRESSION_WITH_VALUE_VECTOR(EXPRESSION)  case PredicateType::EXPRESSION: \
  if (_field_name.size() > 0)                                           \
    return new EXPRESSION<ValueType>(_input_index, _field_name, _value); \
  else                                                                  \
    return new EXPRESSION<ValueType>(_input_index, _field, _value); \
  break;

#define GENERATE_GENERIC_EXPRESSION(NAME, EXPRESSION, OPERATOR)  case PredicateType::NAME: \
  if (_field_name.size() > 0)                                           \
    return new EXPRESSION<ValueType, OPERATOR<ValueType> >(_input_index, _field_name, json_converter::convert<ValueType>(_value)); \
  else                                                                  \
    return new EXPRESSION<ValueType, OPERATOR<ValueType> >(_input_index, _field, json_converter::convert<ValueType>(_value)); \
  break;

namespace hyrise {
namespace access {

struct expression_factory {
  typedef SimpleFieldExpression *value_type;

  size_t _input_index;
  field_t _field;
  field_name_t _field_name;
  PredicateType::type _predicate;
  Json::Value _value;

  expression_factory() {}

  expression_factory(size_t input_index,
                     field_t field_index,
                     PredicateType::type predicate,
                     Json::Value value) :
      _input_index(input_index),
      _field(field_index),
      _field_name(""),
      _predicate(predicate),
      _value(value)
  {}

  expression_factory(size_t input_index,
                     field_name_t fieldName,
                     PredicateType::type predicate,
                     Json::Value value) :
      _input_index(input_index),
      _field_name(fieldName),
      _predicate(predicate),
      _value(value)
  {}

  template <typename ValueType>
  value_type operator()() {
    switch (_predicate) {
      GENERATE_EXPRESSION(EqualsExpression);
      GENERATE_EXPRESSION(LessThanExpression);
      GENERATE_EXPRESSION(GreaterThanExpression);
      GENERATE_EXPRESSION(EqualsExpressionRaw);
      GENERATE_EXPRESSION(LessThanExpressionRaw);
      GENERATE_EXPRESSION(GreaterThanExpressionRaw);
      GENERATE_EXPRESSION_OF_TYPE(LikeExpression, hyrise_string_t);
      GENERATE_EXPRESSION_WITH_VALUE_VECTOR(InExpression);

      GENERATE_GENERIC_EXPRESSION(EqualsExpressionValue, GenericExpressionValue, std::equal_to);
      GENERATE_GENERIC_EXPRESSION(LessThanExpressionValue, GenericExpressionValue, std::less);
      GENERATE_GENERIC_EXPRESSION(GreaterThanExpressionValue, GenericExpressionValue, std::greater);
      GENERATE_GENERIC_EXPRESSION(LessThanEqualsExpressionValue, GenericExpressionValue, std::less_equal);
      GENERATE_GENERIC_EXPRESSION(GreaterThanEqualsExpressionValue, GenericExpressionValue, std::greater_equal);
      default:
        throw std::runtime_error("Expression Type not supported");
    }
  }
};
} /*namespace access*/
} /*namessace hyrise*/

