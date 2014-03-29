// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "io/shortcuts.h"
#include "storage/GroupkeyIndex.h"



namespace hyrise {
namespace storage {

class GroupkeyTest : public ::hyrise::Test {};

void print_difference(std::vector<pos_t>& result_actual, std::vector<pos_t>& result_expected) {
  std::cout << "ERROR: actual list of positions does not match expected." << std::endl;
  std::cout << "Actual: ";
  for (auto v : result_actual)
    std::cout << v << ",";
  std::cout << std::endl << "Expected: ";
  for (auto v : result_expected)
    std::cout << v << ",";
  std::cout << std::endl;
}

bool check_equality(PositionRange result_actual, std::vector<pos_t> result_expected) {

  // copy and sort both to make comparison easier
  std::vector<pos_t> result_actual_sorted;
  std::copy(result_actual.cbegin(), result_actual.cend(), std::back_inserter(result_actual_sorted));
  std::sort(result_actual_sorted.begin(), result_actual_sorted.end());
  std::sort(result_expected.begin(), result_expected.end());

  if (result_actual_sorted.size() != result_expected.size()) {
    print_difference(result_actual_sorted, result_expected);
    return false;
  }

  size_t i = 0;
  for (auto& value : result_actual_sorted) {
    if (value != result_expected[i++]) {
      print_difference(result_actual_sorted, result_expected);
      return false;
    }
  }

  return true;
}


TEST_F(GroupkeyTest, getEqual) {

  std::shared_ptr<AbstractTable> t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  GroupkeyIndex<hyrise_int_t> index(t, 0);

  ASSERT_TRUE(check_equality(index.getPositionsForKey(0), {0}));
  ASSERT_TRUE(check_equality(index.getPositionsForKey(200), {1}));
  ASSERT_TRUE(check_equality(index.getPositionsForKey(400), {2}));
  ASSERT_TRUE(check_equality(index.getPositionsForKey(6), {3}));
  ASSERT_TRUE(check_equality(index.getPositionsForKey(8), {4}));

  // not existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKey(1), {}));
  ASSERT_TRUE(check_equality(index.getPositionsForKey(300), {}));
  ASSERT_TRUE(check_equality(index.getPositionsForKey(800), {}));
}

TEST_F(GroupkeyTest, getLessThan) {

  std::shared_ptr<AbstractTable> t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  GroupkeyIndex<hyrise_int_t> index(t, 0);

  // existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLT(0), {}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLT(6), {0}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLT(8), {0, 3}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLT(200), {0, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLT(400), {0, 1, 3, 4}));

  // not existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLT(100), {0, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLT(800), {0, 1, 2, 3, 4}));
}

TEST_F(GroupkeyTest, getLessThanEqual) {

  std::shared_ptr<AbstractTable> t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  GroupkeyIndex<hyrise_int_t> index(t, 0);

  // existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLTE(0), {0}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLTE(6), {0, 3}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLTE(8), {0, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLTE(200), {0, 1, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLTE(400), {0, 1, 2, 3, 4}));

  // not existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLTE(100), {0, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyLTE(800), {0, 1, 2, 3, 4}));
}


TEST_F(GroupkeyTest, getGreaterThan) {

  std::shared_ptr<AbstractTable> t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  GroupkeyIndex<hyrise_int_t> index(t, 0);

  // existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(0), {1, 2, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(6), {1, 2, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(8), {1, 2}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(200), {2}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(400), {}));

  // not existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(100), {1, 2}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(800), {}));
}

TEST_F(GroupkeyTest, getGreaterThanEqual) {

  std::shared_ptr<AbstractTable> t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  GroupkeyIndex<hyrise_int_t> index(t, 0);

  // existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGTE(0), {0, 1, 2, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGTE(6), {1, 2, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGTE(8), {1, 2, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGTE(200), {1, 2}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGTE(400), {2}));

  // not existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(100), {1, 2}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyGT(800), {}));
}

TEST_F(GroupkeyTest, getBetween) {

  std::shared_ptr<AbstractTable> t = io::Loader::shortcuts::load("test/lin_xxxs.tbl");
  GroupkeyIndex<hyrise_int_t> index(t, 0);

  // existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyBetween(0, 400), {0, 1, 2, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyBetween(200, 800), {1, 2}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyBetween(200, 6), {1, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyBetween(6, 200), {1, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyBetween(200, 400), {1, 2}));

  // not existing values
  ASSERT_TRUE(check_equality(index.getPositionsForKeyBetween(1, 300), {1, 3, 4}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyBetween(300, 350), {}));
  ASSERT_TRUE(check_equality(index.getPositionsForKeyBetween(0, 800), {0, 1, 2, 3, 4}));
}
}
}
