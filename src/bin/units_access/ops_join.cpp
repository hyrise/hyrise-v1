// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <string>
#include "helper.h"
#include <access.h>
#include <storage.h>


#include <io/shortcuts.h>

namespace hyrise {
namespace access {

class JoinTests : public AccessTest {};

TEST_F(JoinTests, DISABLED_join_column_renaming) {
  const auto& table1 = io::Loader::shortcuts::load("test/join_transactions.tbl");
  const auto& table2 = io::Loader::shortcuts::load("test/join_transactions_2.tbl");

  auto join = std::make_shared<JoinScan>(JoinType::EQUI);
  join->addInput(table1);
  join->addInput(table2);
  join->addCombiningClause(AND);
  join->addJoinClause<int>(0, 0, 1, 0);
  join->addJoinClause<std::string>(0, 1, 1, 1);
  const auto& table1_join_table2 = join->execute()->getResultTable();

  ASSERT_EQ(table1_join_table2->metadataAt(0).getName(), std::string("year") + RENAMED_COLUMN_APPENDIX_LEFT);
  ASSERT_EQ(table1_join_table2->metadataAt(1).getName(), std::string("currency") + RENAMED_COLUMN_APPENDIX_LEFT);
  ASSERT_EQ(table1_join_table2->metadataAt(2).getName(), std::string("amount") + RENAMED_COLUMN_APPENDIX_LEFT);
  ASSERT_EQ(table1_join_table2->metadataAt(3).getName(), std::string("year") + RENAMED_COLUMN_APPENDIX_RIGHT);
  ASSERT_EQ(table1_join_table2->metadataAt(4).getName(), std::string("currency") + RENAMED_COLUMN_APPENDIX_RIGHT);
  ASSERT_EQ(table1_join_table2->metadataAt(5).getName(), std::string("amount") + RENAMED_COLUMN_APPENDIX_RIGHT);
  ASSERT_EQ(table1_join_table2->metadataAt(6).getName(), "testcolumn");

}

TEST_F(JoinTests, join_exchange_rates) {
  storage::c_atable_ptr_t table1 = io::Loader::shortcuts::load("test/join_transactions.tbl");
  storage::c_atable_ptr_t table2 = io::Loader::shortcuts::load("test/join_exchange.tbl");

  auto expr4 = new EqualsExpression<std::string>(table2, 2, "USD");

  auto scan = std::make_shared<SimpleTableScan>();
  scan->addInput(table2);
  scan->setProducesPositions(true);
  scan->setPredicate(expr4);

  const auto& filtered_rates = scan->execute()->getResultTable();

  auto join = std::make_shared<hyrise::access::JoinScan>(hyrise::access::JoinType::EQUI);
  join->addInput(table1);
  join->addInput(filtered_rates);
  join->addCombiningClause(AND);
  join->addJoinClause<int>(0, 0, 1, 0);
  join->addJoinClause<std::string>(0, 1, 1, 1);


  const auto& out = join->execute()->getResultTable();

  const auto& reference = io::Loader::shortcuts::load("test/reference/join_exchange_rates.tbl");
  ASSERT_TRUE(out->contentEquals(reference));
}

TEST_F(JoinTests, hash_join_exchange_rates_multiple_columns) {
  storage::c_atable_ptr_t table1 = io::Loader::shortcuts::load("test/join_transactions.tbl");
  storage::c_atable_ptr_t table2 = io::Loader::shortcuts::load("test/join_exchange.tbl");

  auto expr4 = new EqualsExpression<std::string>(table2, 2, "USD");

  auto scan = std::make_shared<SimpleTableScan>();
  scan->addInput(table2);
  scan->setProducesPositions(true);
  scan->setPredicate(expr4);

  const auto& filtered_rates = scan->execute()->getResultTable();


  auto hashBuild1 = std::make_shared<hyrise::access::HashBuild>();
  hashBuild1->addInput(filtered_rates);
  hashBuild1->addField(0);
  hashBuild1->addField(1);
  hashBuild1->setKey("join");
  auto hashedFilteredRates1 = hashBuild1->execute()->getResultHashTable();

  auto hashJoinProbe = std::make_shared<hyrise::access::HashJoinProbe>();
  hashJoinProbe->addInput(table1);
  hashJoinProbe->addField(0);
  hashJoinProbe->addField(1);
  hashJoinProbe->addInput(hashedFilteredRates1);
  auto result = hashJoinProbe->execute()->getResultTable();


  const auto& reference = io::Loader::shortcuts::load("test/reference/join_exchange_rates.tbl");
  ASSERT_TRUE(result->contentEquals(reference));

}

}
}

