// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <gtest/gtest.h>

namespace hyrise {

class Test : public ::testing::Test {
 public:
  virtual ~Test() {};
  virtual void SetUp() {}
  virtual void TearDown() {}
};

} // namespace hyrise

