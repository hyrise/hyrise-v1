// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_TESTING_MAIN_H_
#define SRC_LIB_TESTING_MAIN_H_

/// Generic main method that can be used in testing binaries
/// Sets up log4cxx logging and adds "--minimal" flag to binary
int minimalistMain(int argc, char **argv);


#endif  // SRC_LIB_TESTING_MAIN_H_
