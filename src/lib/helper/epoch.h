// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_HELPER_EPOCH_H_
#define SRC_LIB_HELPER_EPOCH_H_

#include <stdint.h>
typedef uint64_t epoch_t; // time in nanoseconds since some undefined starting point (use differences only)
epoch_t get_epoch_nanoseconds();

#endif  // SRC_LIB_HELPER_EPOCH_H_
