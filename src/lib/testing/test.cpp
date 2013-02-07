// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

::testing::AssertionResult AssertTableContentEquals(const char *left_exp,
                                                    const char *right_exp,
                                                    const hyrise::storage::c_atable_ptr_t left,
                                                    const hyrise::storage::c_atable_ptr_t right) {
  if (left->contentEquals(right)) {
    return ::testing::AssertionSuccess();
  }

  left->print();
  right->print();
  return ::testing::AssertionFailure() << "The content of " << left_exp << " does not equal the content of " << right_exp;
}