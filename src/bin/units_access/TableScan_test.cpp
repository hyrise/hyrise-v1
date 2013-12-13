// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "access/TableScan.h"
#include "access/expressions/pred_EqualsExpression.h"
#include "access/expressions/pred_CompoundExpression.h"
#include "io/shortcuts.h"
#include "access/Barrier.h"
#include "helper/make_unique.h"

namespace hyrise { namespace access {

TEST(TableScan, test) {
  auto tbl = io::Loader::shortcuts::load("test/tables/companies.tbl");
  TableScan ts(std::unique_ptr<EqualsExpression<hyrise_int_t> >(new EqualsExpression<hyrise_int_t>(0, 0, 1)));
  ts.addInput(tbl);
  const auto& result = ts.execute()->getResultTable();
  ASSERT_EQ(1u, result->size());
}

TEST(TableScan, test2) {
  auto eq = std::unique_ptr<EqualsExpression<hyrise_string_t> >(new EqualsExpression<hyrise_string_t>(0, 1, "Apple Inc"));
  auto tbl = io::Loader::shortcuts::load("test/tables/companies.tbl");
  TableScan ts(std::move(eq));
  ts.addInput(tbl);
  const auto& result = ts.execute()->getResultTable();
  ASSERT_EQ(1u, result->size());
}

TEST(TableScan, testDynamicParallelization) {
  auto MTS = 20;

  auto tbl = io::Loader::shortcuts::load("test/tables/companies.tbl");

  auto eq = make_unique<EqualsExpression<hyrise_string_t>>(0, 1, "Apple Inc");
  auto resizedTbl = tbl->copy_structure();
  resizedTbl->resize(100000); // 100k
  auto fakeTask = std::make_shared<Barrier>();
  fakeTask->addInput(resizedTbl);
  fakeTask->addField(0);
  auto ts = std::make_shared<TableScan>(std::move(eq));
  ts->addDependency(fakeTask);
  (*fakeTask)();
  auto dynamicCount1 = ts->determineDynamicCount(MTS);

  auto eq2 = make_unique<EqualsExpression<hyrise_string_t>>(0, 1, "Apple Inc");
  auto resizedTbl2 = tbl->copy_structure();
  resizedTbl2->resize(10000000); // 10 million
  auto fakeTask2 = std::make_shared<Barrier>();
  fakeTask2->addInput(resizedTbl2);
  fakeTask2->addField(0);
  auto ts2 = std::make_shared<TableScan>(std::move(eq2));
  ts2->addDependency(fakeTask2);
  (*fakeTask2)();
  auto dynamicCount2 = ts2->determineDynamicCount(MTS);
  
  ASSERT_GT(dynamicCount2, dynamicCount1); 
}

}}
