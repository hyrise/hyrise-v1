// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/GetLastRecords.h"
#include "access/system/QueryParser.h"
#include "storage/TableRangeView.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<GetLastRecords>("GetLastRecords");
}

void GetLastRecords::executePlanOperation() {
  const auto& table = input.getTable(0);
  size_t end = table->size();
  storage::atable_ptr_t result;
  // if table has less rows than _recors, return table
  if (end <= _records)
    result = std::const_pointer_cast<storage::AbstractTable>(table);
  else {
    size_t start = end - _records;
    result = storage::TableRangeView::create(std::const_pointer_cast<storage::AbstractTable>(table), start, end);
  }
  addResult(result);
}

std::shared_ptr<PlanOperation> GetLastRecords::parse(const Json::Value& data) {
  auto op = BasicParser<GetLastRecords>::parse(data);
  op->_records = data["records"].asUInt();
  return op;
}
}
}
