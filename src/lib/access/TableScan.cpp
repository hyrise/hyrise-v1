#include "access/TableScan.h"

#include "access/SpecialExpression.h"
#include "access/pred_SimpleExpression.h"
#include "storage/AbstractTable.h"
#include "storage/PointerCalculator.h"
#include "storage/PointerCalculatorFactory.h"
#include "helper/types.h"


namespace hyrise { namespace access {

TableScan::TableScan(SimpleExpression* expr) : _expr(expr) {}

TableScan::~TableScan() {
  delete(_expr);
}

void TableScan::setupPlanOperation() {
  const auto& table = getInputTable();
  _expr->walk({table});
}

void TableScan::executePlanOperation() {
  pos_list_t* positions = _expr->match(0, getInputTable()->size());
  addResult(PointerCalculatorFactory::createPointerCalculatorNonRef(getInputTable(), nullptr, positions));
}

}}
