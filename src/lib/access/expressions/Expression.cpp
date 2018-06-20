#include "access/expressions/Expression.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace access {

pos_list_t* Expression::evaluate() {
  auto results = new pos_list_t;

  evaluateMain(results);
  if (deltaExists())
    evaluateDelta(results);

  return results;
}
}
}