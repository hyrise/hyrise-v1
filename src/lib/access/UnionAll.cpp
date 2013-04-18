#include "access/UnionAll.h"

#include "storage/PointerCalculator.h"
#include "storage/HorizontalTable.h"
namespace hyrise { namespace access {

namespace { auto _ = QueryParser::registerTrivialPlanOperation<UnionAll>("UnionAll"); }

void UnionAll::executePlanOperation() {
  const auto& tables = input.getTables();
  std::vector<std::shared_ptr<const PointerCalculator>> pcs(tables.size());
  std::transform(begin(tables), end(tables),
                 begin(pcs),
                 [] (decltype(*begin(tables)) table) {
                   return std::dynamic_pointer_cast<const PointerCalculator>(table);
                 });
  if (std::all_of(begin(pcs), end(pcs), [] (decltype(*begin(tables)) pc) { return pc != nullptr; })) {
    addResult(PointerCalculator::concatenate_many(begin(pcs), end(pcs)));
  } else {
    addResult(std::make_shared<const HorizontalTable>(tables));
  }
}

}}
