#include <gtest/gtest.h>
#include <gtest/gtest-bench.h>
#include <helper/PapiTracer.h>
#include <io.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Gets hold of the event listener list.
  ::testing::TestEventListeners &listeners =
        ::testing::UnitTest::GetInstance()->listeners();

  delete listeners.Release(listeners.default_result_printer());
  listeners.Append(new testing::BenchmarkPrinter);

  // get xml printer and remove it, we have to add our own here
  // listeners.Release(listeners.default_xml_generator());

  /*// Adds a listener to the end.  Google Test takes the ownership.
  if (argc == 3) {
    if (strcmp(argv[1], "--toCodespeed") == 0) {
      listeners.Append(new testing::BenchmarkPublisher(argv[2]));
    } else {
      std::cout << "Bad argument!" << std::endl;
      exit(EXIT_FAILURE);
    }
  } else {
    
    }*/

  //preload environment

  if (getenv("HYRISE_DB_PATH") == nullptr) {
    std::cout << "HYRISE_DB_PATH environment variable is not set!" << std::endl;
    exit(EXIT_FAILURE);
  }

  return RUN_ALL_TESTS();
}
