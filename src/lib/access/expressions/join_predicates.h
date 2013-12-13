// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>
#include "expression_types.h"

namespace hyrise {
namespace access {

/*
 * @brief Basice Join Expression like left.a == right.b
 */
class JoinExpression {
 public:
  JoinExpression() { }
  virtual ~JoinExpression() { }

  virtual void walk(const std::vector<storage::c_atable_ptr_t > &i) = 0;

  inline virtual bool operator()(size_t left_row, size_t right_row) {
    throw std::runtime_error("Cannot call base class");
  }
};

/*
 * @brief compound expression to allow multiple join expressions in
 * one clause
 */
class CompoundJoinExpression : public JoinExpression {

  ExpressionType type;

 public:

  JoinExpression *lhs;

  JoinExpression *rhs;

  virtual ~CompoundJoinExpression() {
    delete lhs;
    delete rhs;
  }


  CompoundJoinExpression(JoinExpression *_lhs, JoinExpression *_rhs, ExpressionType _type) : type(_type), lhs(_lhs), rhs(_rhs) { }

  explicit CompoundJoinExpression(ExpressionType t): type(t), lhs(nullptr), rhs(nullptr)
  {}

  virtual void walk(const std::vector<storage::c_atable_ptr_t > &i) {
    lhs->walk(i);
    rhs->walk(i);
  }

  inline virtual bool operator()(size_t left_row, size_t right_row) {
    switch (type) {
      case AND:
        return (*lhs)(left_row, right_row) && (*rhs)(left_row, right_row);
        break;

      case OR:
        return (*lhs)(left_row, right_row) || (*rhs)(left_row, right_row);
        break;

      default:
        throw std::runtime_error("Bad Expression Type");
        break;
    }
  }
};

/*
 * @brief Equals Expression to be used by joins
 *
 * Type based join expression based on two tables. Either the tables
 * are directly set when constructing the object or they are later on
 * injected during the query execution.
 */
template <typename T>
class EqualsJoinExpression : public JoinExpression {
 private:
  storage::c_atable_ptr_t left;
  storage::c_atable_ptr_t right;

  field_t left_field;
  field_t right_field;

  field_name_t _left_field_name;
  field_name_t _right_field_name;

  size_t left_input;
  size_t right_input;

 public:

  EqualsJoinExpression(size_t l, field_t _left, size_t r, field_t right):
      left_field(_left), right_field(right), left_input(l),
      right_input(r)
  {}

  EqualsJoinExpression(size_t l, field_name_t _left, size_t r, field_name_t right):
      _left_field_name(_left), _right_field_name(right), left_input(l),
      right_input(r)
  {}

  EqualsJoinExpression(storage::c_atable_ptr_t _left,
                       field_t _left_field,
                       storage::c_atable_ptr_t _right,
                       field_t _right_field) :
      left(_left), right(_right), left_field(_left_field), right_field(_right_field) { }


  ~EqualsJoinExpression() {
  }

  virtual void walk(const std::vector<storage::c_atable_ptr_t > &i) {
    left = i[left_input];
    right = i[right_input];

    if (_left_field_name.size() > 0) left_field = left->numberOfColumn(_left_field_name);
    if (_right_field_name.size() > 0) right_field = right->numberOfColumn(_right_field_name);
  }

  inline virtual bool operator()(size_t left_row, size_t right_row) {
    return left->getValue<T>(left_field, left_row) == right->getValue<T>(right_field, right_row);
  }

  static EqualsJoinExpression<T> *parse(const Json::Value &value) {
    if (value["field_right"].isNumeric()) {
      return new EqualsJoinExpression<T>(value["input_left"].asUInt(),
                                         value["field_left"].asUInt(),
                                         value["input_right"].asUInt(),
                                         value["field_right"].asUInt());
    }
    if (value["field_right"].isString())  {
      return new EqualsJoinExpression<T>(value["input_left"].asUInt(),
                                         value["field_left"].asString(),
                                         value["input_right"].asUInt(),
                                         value["field_right"].asString());
    }
    throw std::runtime_error("Failed to parse EqualsExpression");
  }
};

} } // namespace hyrise::access

