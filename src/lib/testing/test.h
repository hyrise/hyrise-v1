// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_TESTING_TEST_H_
#define SRC_LIB_TESTING_TEST_H_

#include <algorithm>

#include "gtest/gtest.h"
#include "testing/base_test.h"
#include "io/StorageManager.h"

#include "helper/types.h"
#include "helper/stringhelpers.h"

template<typename It1, typename It2>
::testing::AssertionResult contains_all(const It1 a, const It2 b) {
  //static_assert(std::is_same<typename It1::value_type, typename It2::value_type>::value == true);
  typedef typename It1::value_type vt;
  std::vector<vt> unmatched_items;
  for (const auto & item: b) {
    if (std::find(begin(a), end(a), item) == end(a)) {
      unmatched_items.push_back(item);
    }
  }
  if (unmatched_items.size() == 0)
    return ::testing::AssertionSuccess();
  else
    return ::testing::AssertionFailure() << "Missing elements " << joinString(unmatched_items, ",");
}


::testing::AssertionResult AssertTableContentEquals(const char *left_exp,
                                                    const char *right_exp,
                                                    const hyrise::storage::c_atable_ptr_t left,
                                                    const hyrise::storage::c_atable_ptr_t right);

#define ASSERT_TABLE_EQUAL(a,b) EXPECT_PRED_FORMAT2(AssertTableContentEquals, a, b)



namespace hyrise {

class StorageManagerTest : public Test {
 public:
  virtual ~StorageManagerTest() {};

  virtual void SetUp() {
    StorageManager *sm = StorageManager::getInstance();
    sm->removeAll();
  }

  virtual void TearDown() {
  }
};

namespace access {

class AccessTest : public Test {
 public:
  virtual void SetUp() {
    StorageManager *sm = StorageManager::getInstance();
    sm->removeAll();
  }

  virtual void TearDown() {
  }
};

}
}
#endif  // SRC_LIB_TESTING_TEST_H_
