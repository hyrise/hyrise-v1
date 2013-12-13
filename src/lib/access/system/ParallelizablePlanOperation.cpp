#include "access/system/ParallelizablePlanOperation.h"

#include "storage/TableRangeView.h"

namespace hyrise {  namespace access {

std::pair<std::uint64_t, std::uint64_t> ParallelizablePlanOperation::distribute(
    const std::uint64_t numberOfElements,
    const std::size_t part,
    const std::size_t count) {
  auto elementsPerPart = numberOfElements / count;
  auto first = elementsPerPart * part;
  auto last = (part + 1 == count) ? numberOfElements : elementsPerPart * (part + 1);
  return {first, last};
}

void ParallelizablePlanOperation::splitInput() {
  const auto& tables = input.getTables();
  if (_count > 0 && !tables.empty()) {
    auto r = distribute(tables[0]->size(), _part, _count);
    input.setTable(storage::TableRangeView::create(std::const_pointer_cast<storage::AbstractTable>(tables[0]), r.first, r.second), 0);
  }
}


void ParallelizablePlanOperation::refreshInput() {
  PlanOperation::refreshInput();
  splitInput();
}

void ParallelizablePlanOperation::setPart(size_t part) {
    _part = part;
}
void ParallelizablePlanOperation::setCount(size_t count) {
  _count = count;
}

}}
