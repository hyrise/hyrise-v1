// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "pred_common.h"
#include <vector>
#include <algorithm>
#include <json.h>
#include "../json_converters.h"
#include "helper/stringhelpers.h"

namespace hyrise {
namespace access {

///
/// IN Expression for comparing a column to multiple values
/// "value" is an array containing the values
/// Example :
///   "select" : {
///         "type" : "SimpleTableScan",
///         "predicates":[{ "type" : "IN", "in" : 0, "f" : "quarter", "vtype" : 0, "value": [2, 3, 4] }]
///     }
/// This is equal to the mysql syntax: SELECT * FROM table WHERE quarter IN(2,3,4);
///
template <typename T>
class InExpression : public SimpleFieldExpression {
public:
  InExpression(size_t i, field_t f, const Json::Value& value):
    SimpleFieldExpression(i, f),
    values(getValues(value))
  {}

  InExpression(size_t i, field_name_t f, const Json::Value& value):
    SimpleFieldExpression(i, f),
    values(getValues(value))
  {}

  InExpression(storage::c_atable_ptr_t _table, field_t _field, const Json::Value& value):
    SimpleFieldExpression(_table, _field),
    values(getValues(value))
  {}

  ///
  /// @return true if the value at column[field,row] matches any values of the list named "values"
  ///
  inline virtual bool operator()(size_t row) {
    T currentValue = table->getValue<T>(field, row);
    return std::find(values.cbegin(), values.cend(), currentValue) != values.cend();
  }

private:
  const std::vector<T> values;
  ///
  /// converts the string containing the values to a vector of values of the right type
  /// @return list of values to compare
  ///
  const std::vector<T> getValues(const Json::Value & values) const {
    if (!values.isArray())
      throw std::runtime_error("IN Expression value must be an array.");

    std::vector<T> converted;

    for (const auto &v : values) {
      converted.push_back(json_converter::convert<T>(v));
    }
    return converted;
  }
};

} } // namespace hyrise::access

