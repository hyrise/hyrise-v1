// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <boost/regex.hpp>

#include "pred_common.h"
#include <helper/types.h>
#include <boost/regex.hpp>

namespace hyrise {
namespace access {

/// LIKE expression for string comparison using regular expressions.

/// Set vtype to string (2) when using in json.
/// Converts the value string to boost::regex.
/// When you want to have characters in your value string that are part of the regex syntax,
/// but that should not be converted, escape them with double bachslash ("\\"). \n
/// Json Example: \n
/// { "type" : "LIKE", "in" : 0, "f" : "employee_name", "vtype" : 2, "value": "Jeffre. .\\. .*ley" } \n
/// matches employee_name "Jeffrey O. Henley".
///
class LikeExpression : public SimpleFieldExpression {
public:
  LikeExpression(size_t i, field_t f, const hyrise_string_t& value) :
    SimpleFieldExpression(i, f),
    regExpr(boost::regex(value))
  { }

  LikeExpression(size_t i, field_name_t f, const hyrise_string_t& value) :
    SimpleFieldExpression(i, f),
    regExpr(boost::regex(value))
  { }

  LikeExpression(const storage::c_atable_ptr_t& _table, field_t _field, const hyrise_string_t& value) :
    SimpleFieldExpression(_table, _field),
    regExpr(boost::regex(value))
  { }

  ///
  /// Applies the like expression on each field using the generated regex object.
  /// @return true if current line matches the regular expression.
  ///
  inline virtual bool operator()(size_t row) {
    std::string currentValue = std::string(table->getValue<hyrise_string_t>(field, row));

    return boost::regex_match(currentValue, regExpr);
  }

private:
  /// Hold the regular expression object. Generated in constructor.
  const boost::regex regExpr;
};

} } // namesapce hyrise::access

