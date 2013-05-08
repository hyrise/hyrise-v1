// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "testing/test.h"

#include "io/shortcuts.h"
#include "access/UnionAll.h"
#include "storage/HorizontalTable.h"
#include "storage/PointerCalculator.h"

namespace hyrise { namespace access {

TEST(UnionAllTests, basic_union_scan_test) {
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");

  UnionAll us;
  us.addInput(t);
  us.addInput(t);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_EQ(2* t->size(), result->size());
}


TEST(UnionAllTests, pointer_calc_input) {
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");

  auto pc1 = std::make_shared<PointerCalculator>(t, nullptr, nullptr);
  auto pc2 = std::make_shared<PointerCalculator>(t, nullptr, nullptr);

  UnionAll us;
  us.addInput(pc1);
  us.addInput(pc2);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_TRUE(std::dynamic_pointer_cast<const PointerCalculator>(result) != nullptr);
}


TEST(UnionAllTests, many_pointer_calc_input) {
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");

  auto pc1 = std::make_shared<PointerCalculator>(t, nullptr, nullptr);
  auto pc2 = std::make_shared<PointerCalculator>(t, nullptr, nullptr);
  auto pc3 = std::make_shared<PointerCalculator>(t, nullptr, nullptr);

  UnionAll us;
  us.addInput(pc1);
  us.addInput(pc2);
  us.addInput(pc3);
  us.execute();

  const auto &result = us.getResultTable();
  ASSERT_EQ(3 * t->size(), result->size()) << "Needs to be three times as large as original table";
  ASSERT_TRUE(std::dynamic_pointer_cast<const PointerCalculator>(result) != nullptr);
}

TEST(UnionAllTests, mixed_input) {
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");

  auto pc1 = std::make_shared<PointerCalculator>(t, nullptr, nullptr);
  auto pc2 = std::make_shared<PointerCalculator>(t, nullptr, nullptr);

  UnionAll us;
  us.addInput(pc1);
  us.addInput(pc2);
  us.addInput(t);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_TRUE(std::dynamic_pointer_cast<const HorizontalTable>(result) != nullptr);
}


}
}
