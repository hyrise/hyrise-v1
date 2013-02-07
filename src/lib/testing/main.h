#ifndef SRC_LIB_TESTING_MAIN_H_
#define SRC_LIB_TESTING_MAIN_H_

namespace hyrise {
namespace testing {

/// Generic main method that can be used in testing binaries
/// Sets up log4cxx logging and adds "--minimal" flag to binary
int minimalistMain(int argc, char **argv);

}
}

#endif  // SRC_LIB_TESTING_MAIN_H_
