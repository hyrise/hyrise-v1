// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "pred_buildExpression.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "expression_types.h"
#include "../json_converters.h"
#include "predicates.h"
#include "pred_expression_factory.h"
#include "storage/meta_storage.h"

SimpleFieldExpression *buildFieldExpression(PredicateType::type pred_type, const rapidjson::Value &predicate) {
  hyrise::storage::type_switch<hyrise_basic_types> ts;

  if (predicate["f"].isNumeric()) {
    auto fun = hyrise::access::expression_factory(predicate["in"].asUInt(), predicate["f"].asUInt(), pred_type, predicate["value"]);
    return ts(predicate["vtype"].asUInt(), fun);
  } else if (predicate["f"].isString()) {
    auto fun = hyrise::access::expression_factory(predicate["in"].asUInt(), predicate["f"].asString(), pred_type, predicate["value"]);
    return ts(predicate["vtype"].asUInt(), fun);
  }
};

SimpleExpression *buildExpression(const rapidjson::Value &predicates) {
  PredicateBuilder b;
  PredicateType::type pred_type;

  for (unsigned i = 0; i < predicates.size(); ++i) {
    const auto& predicate = predicates[i];

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
