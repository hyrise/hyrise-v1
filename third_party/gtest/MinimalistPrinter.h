// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_TESTING_MINIMALISTPRINTER_H_
#define SRC_LIB_TESTING_MINIMALISTPRINTER_H_

#include <memory>

#include "gtest.h"

/// Printer implementation class, to hide implementation details
struct PrinterImpl;

/// Minimalistic printing features for gtest testrunner
class MinimalistPrinter : public ::testing::EmptyTestEventListener {
  std::unique_ptr<PrinterImpl> _impl;

 public:
  MinimalistPrinter();
  typedef ::testing::TestPartResult test_part_t;
  virtual void OnTestStart(const ::testing::TestInfo &test_info);
  virtual void OnTestPartResult(const ::testing::TestPartResult &test_part_result);
  virtual void OnTestEnd(const ::testing::TestInfo &test_info);
  virtual void OnTestProgramEnd(const ::testing::UnitTest &unit);
};



#endif  // SRC_LIB_TESTING_MINIMALISTPRINTER_H_
