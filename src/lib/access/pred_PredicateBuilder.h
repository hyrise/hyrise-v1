#ifndef SRC_LIB_ACCESS_PRED_PREDICATEBUILDER_H_
#define SRC_LIB_ACCESS_PRED_PREDICATEBUILDER_H_
#include <stack>

#include "pred_common.h"
#include "pred_CompoundExpression.h"

/*
 * @brief Creates the predicate tree required for Selections
 *
 * All predicates both SimpleFieldExpression and CompoundExpression
 * have to be added in the correct execution order using prefix
 * notation.
 */

class PredicateBuilder {
  std::stack<CompoundExpression *> previous;
  SimpleExpression *root;

 public:
  PredicateBuilder();
  virtual ~PredicateBuilder();
  void add(SimpleFieldExpression *exp);
  void add(CompoundExpression *exp);
  SimpleExpression *build();
};

#endif  // SRC_LIB_ACCESS_PRED_PREDICATEBUILDER_H_
