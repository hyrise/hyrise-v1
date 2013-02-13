// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/PlanOperation.h"
#include "access/NoOp.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

class ParallelExecutionTest : public AccessTest {};

// test data distribution method applied to PlanOp input
TEST_F(ParallelExecutionTest, data_distribution){

  //void _PlanOperation::distribute(
  //  const u_int64_t numberOfElements,
  //  u_int64_t &first,
  //  u_int64_t &last) const {
  //const u_int64_t
  //    elementsPerPart     = numberOfElements / _count,
  //    remainingElements   = numberOfElements - elementsPerPart * _count,
  //    extraElements       = _part <= remainingElements ? _part : remainingElements,
  //    partsExtraElement   = _part < remainingElements ? 1 : 0;
  //first                   = elementsPerPart * _part + extraElements;
  //last                    = first + elementsPerPart + partsExtraElement - 1;
  //}


  NoOp nop;
  u_int64_t first = 0, last = 0;

  nop.setCount(2);
  nop.setPart(0);
  nop.distribute(2000, first, last);
  EXPECT_EQ(0u, first);
  EXPECT_EQ(999u, last);

  nop.setPart(1);
  nop.distribute(2000, first, last);
  EXPECT_EQ(1000u, first);
  EXPECT_EQ(1999u, last);

  nop.setCount(5);
  nop.setPart(3);
  nop.distribute(5000, first, last);
  EXPECT_EQ(3000u, first);
  EXPECT_EQ(3999u, last);

  nop.setCount(5);
  nop.setPart(0);
  nop.distribute(5002, first, last);
  EXPECT_EQ(0u, first);
  EXPECT_EQ(1000u, last);

  nop.setCount(5);
  nop.setPart(1);
  nop.distribute(5002, first, last);
  EXPECT_EQ(1001u, first);
  EXPECT_EQ(2001u, last);

  nop.setCount(5);
  nop.setPart(2);
  nop.distribute(5002, first, last);
  EXPECT_EQ(2002u, first);
  EXPECT_EQ(3001u, last);

  nop.setCount(5);
  nop.setPart(3);
  nop.distribute(5002, first, last);
  EXPECT_EQ(3002u, first);
  EXPECT_EQ(4001u, last);

  nop.setCount(5);
  nop.setPart(4);
  nop.distribute(5002, first, last);
  EXPECT_EQ(4002u, first);
  EXPECT_EQ(5001u, last);

}

}
}