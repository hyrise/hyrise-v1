// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdint.h>
typedef uint64_t epoch_t; // time in nanoseconds since some undefined starting point (use differences only)
epoch_t get_epoch_nanoseconds();

