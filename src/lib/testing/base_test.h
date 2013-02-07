// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_TESTING_BASE_TEST_H_
#define SRC_LIB_TESTING_BASE_TEST_H_

#include <gtest/gtest.h>

namespace hyrise {
class Test : public ::testing::Test {
 public:
  virtual ~Test() {};
  virtual void SetUp() {}
  virtual void TearDown() {}
};
}

#endif
