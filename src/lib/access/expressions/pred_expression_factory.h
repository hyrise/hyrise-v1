// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PRED_EXPRESSION_FACTORY_H_
#define SRC_LIB_ACCESS_PRED_EXPRESSION_FACTORY_H_

#include "expression_types.h"
#include "../json_converters.h"
#include "predicates.h"

#define GENERATE_EXPRESSION(EXPRESSION)  case PredicateType::EXPRESSION: \
  if (_field_name.size() > 0)                                           \
    return new EXPRESSION<ValueType>(_input_index, _field_name, json_converter::convert<ValueType>(_value)); \
  else                                                                  \
    return new EXPRESSION<ValueType>(_input_index, _field, json_converter::convert<ValueType>(_value)); \
  break;

#define GENERATE_2VALUE_EXPRESSION(EXPRESSION)  case PredicateType::EXPRESSION: \
  if (_field_name.size() > 0)                                           \
    return new EXPRESSION<ValueType>(_input_index, _field_name, json_converter::convert<ValueType>(_value), json_converter::convert<ValueType>(_value2)); \
  else                                                                  \
    return new EXPRESSION<ValueType>(_input_index, _field, json_converter::convert<ValueType>(_value), json_converter::convert<ValueType>(_value2)); \
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
  Json::Value _value2;

  expression_factory() {}

  expression_factory(size_t input_index,
                     field_t field_index,
                     PredicateType::type predicate,
                     Json::Value value,
		     Json::Value value2) :
      _input_index(input_index),
      _field(field_index),
      _field_name(""),
      _predicate(predicate),
      _value(value),
      _value2(value2)
  {}

  expression_factory(size_t input_index,
                     field_name_t fieldName,
                     PredicateType::type predicate,
                     Json::Value value,
                     Json::Value value2) :
      _input_index(input_index),
      _field_name(fieldName),
      _predicate(predicate),
      _value(value),
      _value2(value2)
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

      GENERATE_2VALUE_EXPRESSION(BetweenExpression); 

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
} /*ns access*/
} /*ns hyrise*/

#endif  // SRC_LIB_ACCESS_PRED_EXPRESSION_FACTORY_H_
