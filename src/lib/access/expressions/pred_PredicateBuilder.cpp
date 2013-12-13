// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "pred_PredicateBuilder.h"

namespace hyrise {
namespace access {

PredicateBuilder::PredicateBuilder(): root(nullptr) {
}

PredicateBuilder::~PredicateBuilder() {
}

void PredicateBuilder::add(SimpleFieldExpression *e) {
  if (root == nullptr) {
    root = e;
  } else {
    previous.top()->add(e);
  }

  if (previous.size() > 0 && previous.top()->isSetup()) {
    previous.pop();
  }

}

void PredicateBuilder::add(CompoundExpression *e) {
  if (root == nullptr) {
    root = e;
  } else {
    previous.top()->add(e);
  }

  previous.push(e);
}

SimpleExpression *PredicateBuilder::build() {
  return root;
}

} } // namespace hyrise::access
