// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "pred_buildExpression.h"

#include <json.h>

#include "expression_types.h"
#include "../json_converters.h"
#include "predicates.h"
#include "pred_expression_factory.h"
#include "storage/meta_storage.h"

namespace hyrise {
namespace access {

SimpleFieldExpression *buildFieldExpression(PredicateType::type pred_type, const Json::Value &predicate) {
  storage::type_switch<hyrise_basic_types> ts;
  expression_factory fun;
  if (predicate["f"].isNumeric()) {
    fun = expression_factory(predicate["in"].asUInt(), predicate["f"].asUInt(), pred_type, predicate["value"]);
  } else if (predicate["f"].isString()) {
    fun = expression_factory(predicate["in"].asUInt(), predicate["f"].asString(), pred_type, predicate["value"]);
  }
  return ts(predicate["vtype"].asUInt(), fun);
};

SimpleExpression *buildExpression(const Json::Value &predicates) {
  PredicateBuilder b;

  Json::Value predicate;
  PredicateType::type pred_type;


  for (unsigned i = 0; i < predicates.size(); ++i) {
    predicate = predicates[i];

    pred_type = parsePredicateType(predicate["type"]);

    switch (pred_type) {
      case PredicateType::AND:
        b.add(new CompoundExpression(AND));
        break;

      case PredicateType::OR:
        b.add(new CompoundExpression(OR));
        break;

      case PredicateType::NOT:
        b.add(new CompoundExpression(NOT));
        break;

      default:
        b.add(buildFieldExpression(pred_type, predicate));
    }
  }
  return b.build();
};

} } // namespace hyrise::access

