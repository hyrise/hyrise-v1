// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "helper/demangle.h"
#include <stdlib.h>
#include <cxxabi.h>

#if (__GNUC__ && __cplusplus && __GNUC__ >= 3)
const char* demangle(const char* name) {
  char buf[1024];
  size_t size = 1024;
  int status;
  char* res = abi::__cxa_demangle(name, buf, &size, &status);
  return res;
}
#else
const char* demangle(const char* name) { return name; }
#endif
