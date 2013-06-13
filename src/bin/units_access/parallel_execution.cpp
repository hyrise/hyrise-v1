// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/PlanOperation.h"
#include "access/NoOp.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

class ParallelExecutionTest : public AccessTest {};

// test data distribution method applied to PlanOp input
TEST_F(ParallelExecutionTest, data_distribution){

  NoOp nop;
  u_int64_t first = 0, last = 0;

  nop.setCount(2);
  nop.setPart(0);
  nop.distribute(2000, first, last);
  EXPECT_EQ(0u, first);
  EXPECT_EQ(1000u, last);

  nop.setPart(1);
  nop.distribute(2000, first, last);
  EXPECT_EQ(1000u, first);
  EXPECT_EQ(2000u, last);

  nop.setCount(5);
  nop.setPart(3);
  nop.distribute(5000, first, last);
  EXPECT_EQ(3000u, first);
  EXPECT_EQ(4000u, last);

  nop.setCount(5);
  nop.setPart(0);
  nop.distribute(5002, first, last);
  EXPECT_EQ(0u, first);
  EXPECT_EQ(1000u, last);

  nop.setCount(5);
  nop.setPart(1);
  nop.distribute(5002, first, last);
  EXPECT_EQ(1000u, first);
  EXPECT_EQ(2000u, last);

  nop.setCount(5);
  nop.setPart(2);
  nop.distribute(5002, first, last);
  EXPECT_EQ(2000u, first);
  EXPECT_EQ(3000u, last);

  nop.setCount(5);
  nop.setPart(3);
  nop.distribute(5002, first, last);
  EXPECT_EQ(3000u, first);
  EXPECT_EQ(4000u, last);

  nop.setCount(5);
  nop.setPart(4);
  nop.distribute(5002, first, last);
  EXPECT_EQ(4000u, first);
  EXPECT_EQ(5002u, last);

}

}
}
