#ifndef SRC_BIN_UNITS_IO_HELPER_H_
#define SRC_BIN_UNITS_IO_HELPER_H_

#include <gtest/gtest.h>

#include "helper/types.h"

::testing::AssertionResult AssertTableContentEquals(const char *left_exp,
                                                    const char *right_exp,
                                                    const hyrise::storage::c_atable_ptr_t left,
                                                    const hyrise::storage::c_atable_ptr_t right);

#define ASSERT_TABLE_EQUAL(a,b) EXPECT_PRED_FORMAT2(AssertTableContentEquals, a, b)


#endif // SRC_BIN_UNITS_IO_HELPER_H_
