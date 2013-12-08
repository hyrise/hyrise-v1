// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/HashValueJoin.hpp"
#include "io/shortcuts.h"
#include "storage/MutableVerticalTable.h"
#include "storage/PointerCalculator.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"
namespace hyrise {
namespace access {

class HashValueJoinTests : public AccessTest {};

TEST_F(HashValueJoinTests, basic_hash_value_join_test) {
  auto t1 = io::Loader::shortcuts::load("test/join_transactions.tbl");
  auto t2 = io::Loader::shortcuts::load("test/join_exchange.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/hash_value_join_result.tbl");

  auto hvj = std::make_shared<HashValueJoin<storage::hyrise_int_t>>();
  hvj->setProducesPositions(true);
  hvj->addInput(t1);
  hvj->addField(0);
  hvj->addInput(t2);
  hvj->addField(0);
  hvj->execute();

  const auto &result = hvj->getResultTable();

  EXPECT_RELATION_EQ(result, reference);
}

}
}
