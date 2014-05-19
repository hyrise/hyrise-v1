#include "batvector_expressions.h"

#include "access/expressions/ExpressionRegistration.h"

#include <functional>

namespace hyrise {namespace access {


  namespace {
    auto _eq = Expressions::add<BatVectorExpression<std::equal_to<hyrise_int_t>>>("hyrise::bat::eq");
  }


}}