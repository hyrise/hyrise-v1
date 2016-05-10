// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <gtest/gtest.h>

#include <io/ResourceManager.h>

namespace hyrise {

class Test : public ::testing::Test {
 public:
  virtual ~Test() {};
  virtual void SetUp() {
    io::ResourceManager::getInstance().clear();
  }
  virtual void TearDown() {}
};

}  // namespace hyrise
