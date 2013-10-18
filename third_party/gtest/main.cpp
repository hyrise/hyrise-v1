// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "main.h"

#include <algorithm>
#include <string>
#include <vector>

#include "MinimalistPrinter.h"

#include "gtest.h"
#include "log4cxx/propertyconfigurator.h"

int minimalistMain(int argc, char **argv) {
  setenv("HYRISE_DB_PATH", "test", 1);
  log4cxx::PropertyConfigurator::configure("./build/log_test.properties");

  ::testing::InitGoogleTest(&argc, argv);
  std::vector<std::string> args(argv, argv + argc);
  if (std::find(args.begin(), args.end(), "--minimal") != args.end()) {
    ::testing::TestEventListeners &listeners =
        ::testing::UnitTest::GetInstance()->listeners();
    // Adds a listener to the end.  Google Test takes the ownership.
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new MinimalistPrinter);
  }
  return RUN_ALL_TESTS();
}
