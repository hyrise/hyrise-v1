#include "testing/main.h"

#include <algorithm>
#include <string>
#include <vector>

#include "testing/MinimalistPrinter.h"

#include "gtest/gtest.h"
#include "log4cxx/propertyconfigurator.h"

namespace hyrise {
namespace testing {

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
    listeners.Append(new hyrise::testing::MinimalistPrinter);
  }
  return RUN_ALL_TESTS();
}

}
}
