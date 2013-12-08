#include "access/UnionAll.h"
#include "helper/vector_helpers.h"

#include "storage/PointerCalculator.h"
#include "storage/HorizontalTable.h"

namespace hyrise { namespace access {

namespace { auto _ = QueryParser::registerTrivialPlanOperation<UnionAll>("UnionAll"); }

void UnionAll::executePlanOperation() {
  const auto& tables = input.getTables();
  auto pcs = convert<const storage::PointerCalculator>(tables);
  if (allValid(pcs)) {
    addResult(storage::PointerCalculator::concatenate_many(begin(pcs), end(pcs)));
    return;
  }

  auto mtvs = convert<const storage::MutableVerticalTable>(tables);
  if (allValid(mtvs)) {
    auto left_pcs = functional::collect(mtvs, [] (const std::shared_ptr<const storage::MutableVerticalTable>& mtv) { return std::dynamic_pointer_cast<const storage::PointerCalculator>(mtv->getContainer(0)); });
    auto right_pcs = functional::collect(mtvs, [] (const std::shared_ptr<const storage::MutableVerticalTable>& mtv) { return std::dynamic_pointer_cast<const storage::PointerCalculator>(mtv->getContainer(1)); });


    if (allValid(left_pcs) && allValid(right_pcs)) {
      auto l_concat = storage::PointerCalculator::concatenate_many(std::begin(left_pcs), std::end(left_pcs));
      auto r_concat = storage::PointerCalculator::concatenate_many(std::begin(right_pcs), std::end(right_pcs));
      std::vector<storage::atable_ptr_t> pcs = {l_concat, r_concat};
      addResult(std::make_shared<storage::MutableVerticalTable>(pcs));
      return;
    }
  }
  
  addResult(std::make_shared<const storage::HorizontalTable>(tables));
}

}}
