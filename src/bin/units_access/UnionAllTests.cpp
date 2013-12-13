// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "testing/test.h"

#include "io/shortcuts.h"
#include "access/UnionAll.h"
#include "storage/HorizontalTable.h"
#include "storage/PointerCalculator.h"

namespace hyrise { namespace access {

class UnionAllTests : public Test{
 public:
  storage::atable_ptr_t t;
  void SetUp() {
    t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  }
};
  
TEST_F(UnionAllTests, basic_union_scan_test) {
  UnionAll us;
  us.addInput(t);
  us.addInput(t);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_EQ(2* t->size(), result->size());
}


TEST_F(UnionAllTests, pointer_calc_input) {
  auto pc1 = std::make_shared<storage::PointerCalculator>(t, nullptr, nullptr);
  auto pc2 = std::make_shared<storage::PointerCalculator>(t, nullptr, nullptr);

  UnionAll us;
  us.addInput(pc1);
  us.addInput(pc2);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_TRUE(std::dynamic_pointer_cast<const storage::PointerCalculator>(result) != nullptr);
}


TEST_F(UnionAllTests, many_pointer_calc_input) {
  auto pc1 = std::make_shared<storage::PointerCalculator>(t, nullptr, nullptr);
  auto pc2 = std::make_shared<storage::PointerCalculator>(t, nullptr, nullptr);
  auto pc3 = std::make_shared<storage::PointerCalculator>(t, nullptr, nullptr);

  UnionAll us;
  us.addInput(pc1);
  us.addInput(pc2);
  us.addInput(pc3);
  us.execute();

  const auto &result = us.getResultTable();
  ASSERT_EQ(3 * t->size(), result->size()) << "Needs to be three times as large as original table";
  ASSERT_TRUE(std::dynamic_pointer_cast<const storage::PointerCalculator>(result) != nullptr);
}

TEST_F(UnionAllTests, vertical_nested_pointer_calculators) {
  auto pc1_l = std::make_shared<storage::PointerCalculator>(t, new std::vector<size_t> {1}, new std::vector<size_t> {0, 1});
  auto pc1_r = std::make_shared<storage::PointerCalculator>(t, new std::vector<size_t> {5}, new std::vector<size_t> {2, 3, 4});
  std::vector<storage::atable_ptr_t> pc1 {pc1_l , pc1_r};
  auto mtv1  = std::make_shared<storage::MutableVerticalTable>(pc1);
  
  auto pc2_l = std::make_shared<storage::PointerCalculator>(t, new std::vector<size_t> {2, 3}, new std::vector<size_t> {0, 1});
  auto pc2_r = std::make_shared<storage::PointerCalculator>(t, new std::vector<size_t> {2, 6}, new std::vector<size_t> {2, 3, 4});
  std::vector<storage::atable_ptr_t> pc2 {pc2_l , pc2_r};
  auto mtv2  = std::make_shared<storage::MutableVerticalTable>(pc2);

  UnionAll u;
  u.addInput(mtv1);
  u.addInput(mtv2);
  u.execute();
  
  const auto &result = u.getResultTable();
  ASSERT_TRUE(std::dynamic_pointer_cast<const storage::MutableVerticalTable>(result) != nullptr);
  EXPECT_EQ(3u, result->size());
}

TEST_F(UnionAllTests, mixed_input) {
  auto pc1 = std::make_shared<storage::PointerCalculator>(t, nullptr, nullptr);
  auto pc2 = std::make_shared<storage::PointerCalculator>(t, nullptr, nullptr);

  UnionAll us;
  us.addInput(pc1);
  us.addInput(pc2);
  us.addInput(t);
  us.execute();

  const auto &result = us.getResultTable();

  ASSERT_TRUE(std::dynamic_pointer_cast<const storage::HorizontalTable>(result) != nullptr);
}


}
}
