// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest.h>
#include <gtest/gtest-bench.h>

#include <sys/stat.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Gets hold of the event listener list.
  ::testing::TestEventListeners &listeners =
        ::testing::UnitTest::GetInstance()->listeners();

  delete listeners.Release(listeners.default_result_printer());
  listeners.Append(new testing::BenchmarkPrinter);

  //preload environment

  if (getenv("HYRISE_DB_PATH") == nullptr) {
    std::cout << "HYRISE_DB_PATH environment variable is not set!" << std::endl;
    exit(EXIT_FAILURE);
  }

  return RUN_ALL_TESTS();
}
