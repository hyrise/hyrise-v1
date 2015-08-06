// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <json.h>
#include <string>

#include "helper.h"

#include "testing/test.h"
#include "access/ExpressionScan.h"
#include "access/InsertScan.h"
#include "access/MaterializingScan.h"
#include "access/ProjectionScan.h"
#include "access/SimpleTableScan.h"
#include "access/expressions/predicates.h"
#include "storage/AbstractTable.h"
#include "storage/ColumnMetadata.h"
#include "io/shortcuts.h"
#include "io/TransactionManager.h"


namespace hyrise {
namespace access {

class SelectTests : public AccessTest {
 public:
};

TEST_F(SelectTests, simple_projection_with_position) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  ProjectionScan gs;
  gs.setProducesPositions(true);
  gs.addInput(t);
  gs.addField(0);

  const auto& result = gs.execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_projection.tbl");
  ASSERT_TRUE(result->contentEquals(reference));
}

TEST_F(SelectTests, simple_projection_with_position_mat) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  ProjectionScan gs;
  gs.setProducesPositions(true);
  gs.addInput(t);
  gs.addField(0);

  auto result = gs.execute()->getResultTable();
  MaterializingScan ms(false);
  ms.addInput(result);
  const auto& result2 = ms.execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_projection.tbl");
  ASSERT_TRUE(result2->contentEquals(reference));
}

TEST_F(SelectTests, simple_projection_with_position_all_mat) {
  auto t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  ProjectionScan gs;
  gs.setProducesPositions(true);
  gs.addInput(t);
  gs.addField(0);
  gs.addField(1);

  auto result = gs.execute()->getResultTable();
  MaterializingScan ms(false);
  ms.addInput(result);
  const auto& result2 = ms.execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  ASSERT_TRUE(result2->contentEquals(reference));
}



TEST_F(SelectTests, simple_projection_with_position_mat_memcpy) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  ProjectionScan gs;
  gs.setProducesPositions(true);
  gs.addInput(t);
  gs.addField(0);

  ASSERT_EQ((unsigned)10, t->columnCount());

  auto result = gs.execute()->getResultTable();
  MaterializingScan ms(true);
  ms.addInput(result);
  const auto& result2 = ms.execute()->getResultTable();

  ASSERT_EQ((unsigned)100, result2->size());
  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_projection.tbl");

  ASSERT_TABLE_EQUAL(reference, result2);
}


TEST_F(SelectTests, simple_projection_with_position_mat_and_sample) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  ProjectionScan gs;
  gs.setProducesPositions(true);
  gs.addInput(t);
  gs.addField(0);
  auto result = gs.execute()->getResultTable();

  MaterializingScan ms(false);
  ms.addInput(result);
  ms.setSamples(3);
  const auto& result2 = ms.execute()->getResultTable();

  ASSERT_EQ((unsigned)3, result2->size());
}


TEST_F(SelectTests, simple_projection_with_materialization) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");

  ProjectionScan gs;
  gs.addInput(t);
  gs.setProducesPositions(false);
  gs.addField(0);

  const auto& result = gs.execute()->getResultTable();

  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_projection.tbl");
  ASSERT_TRUE(result->contentEquals(reference));
}

TEST_F(SelectTests, simple_expression) {
  auto t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");

  hyrise::access::ExpressionScan* es = new hyrise::access::ExpressionScan();
  es->addInput(t);
  hyrise::access::AddExp plus(t, 0, 1);
  es->setExpression("plus", &plus);

  const auto& result = es->execute()->getResultTable();

  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_expression.tbl");
  ASSERT_TRUE(result->contentEquals(reference));
}

TEST_F(SelectTests, should_throw_without_predicates) {
  Json::Value v(Json::objectValue);
  v["type"] = "SimpleTableScan";
  ASSERT_THROW(SimpleTableScan::parse(v), std::runtime_error);
}


TEST_F(SelectTests, simple_select) {
  hyrise::storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");

  GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>* expr1 =
      new GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(t, 0, 2009);
  GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>* expr2 =
      new GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(t, 1, 1);
  CompoundExpression* expr3 = new CompoundExpression(expr1, expr2, OR);
  CompoundExpression* expr4 = new CompoundExpression(NOT);
  expr4->lhs = expr3;

  auto scan = std::make_shared<SimpleTableScan>();
  scan->addInput(t);
  scan->setPredicate(expr4);
  scan->setProducesPositions(true);
  const auto& out = scan->execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_select_1.tbl");

  ASSERT_TRUE(out->contentEquals(reference));
}

TEST_F(SelectTests, simple_select_2) {
  hyrise::storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");

  auto* expr5 = new LessThanExpression<hyrise_int_t>(t, 0, 2010);
  auto scan = std::make_shared<SimpleTableScan>();
  scan->addInput(t);
  scan->setPredicate(expr5);
  scan->setProducesPositions(true);

  const auto& out = scan->execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_select_2.tbl");

  ASSERT_TRUE(out->contentEquals(reference));
}


TEST_F(SelectTests, simple_select_3) {
  hyrise::storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");

  GreaterThanExpression<hyrise_int_t>* expr6 = new GreaterThanExpression<hyrise_int_t>(t, 0, 2009);
  GreaterThanExpression<hyrise_int_t>* expr8 = new GreaterThanExpression<hyrise_int_t>(t, 0, 2008);
  CompoundExpression* expr7 = new CompoundExpression(NOT);
  expr7->lhs = expr6;

  CompoundExpression* expr9 = new CompoundExpression(expr8, expr7, AND);
  auto scan = std::make_shared<SimpleTableScan>();
  scan->addInput(t);
  scan->setPredicate(expr9);
  scan->setProducesPositions(true);

  const auto& out = scan->execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/simple_select_3.tbl");

  ASSERT_TRUE(out->contentEquals(reference));
}

TEST_F(SelectTests, select_between) {
  hyrise::storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/groupby_xs.tbl");

  auto stc = std::make_shared<SimpleTableScan>();
  BetweenExpression<hyrise_int_t>* between = new BetweenExpression<hyrise_int_t>(t, t->numberOfColumn("month"), 2, 4);
  stc->addInput(t);
  stc->setPredicate(between);

  const auto& result = stc->execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/select_between.tbl");

  ASSERT_TRUE(result->contentEquals(reference));
}

TEST_F(SelectTests, simple_projection_on_empty_table) {
  hyrise::storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/empty.tbl");

  ProjectionScan gs;
  gs.setProducesPositions(true);
  gs.addInput(t);
  gs.addField(2);
  gs.addField(3);
  gs.addField(4);
  gs.addField(5);
  gs.addField(6);

  const auto& result = gs.execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/empty_after_projection.tbl");
  ASSERT_TRUE(result->contentEquals(reference));
}

TEST_F(SelectTests, select_after_insert_simple) {
  auto ctx = tx::TransactionManager::getInstance().buildContext();
  hyrise::storage::atable_ptr_t s = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto initial_size = s->size();
  hyrise::storage::atable_ptr_t data = s->copy_structure_modifiable(nullptr, s->size());
  data->resize(1);
  for (std::size_t i = 0; i <= 9; ++i) {
    data->setValue<hyrise_int_t>(i, 0, 1000 + i);
  }
  // insert data
  InsertScan isc;
  isc.setTXContext(ctx);
  isc.addInput(s);
  isc.setInputData(data);
  isc.execute();


  ASSERT_EQ(initial_size + 1, isc.getResultTable(0)->size());
}
}
}
