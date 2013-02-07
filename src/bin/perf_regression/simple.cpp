// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest.h>
#include <gtest/gtest-bench.h>
#include <string>

/*class ProjectionBase : public ::testing::Benchmark
  {

  protected:

  int myVal;

  public:

  void SetUp()
  {
  myVal = rand() % 100;
  }

  ProjectionBase()
  {
  SetNumIterations(10);
  SetWarmUp(2);
  }
  };

  BENCHMARK(Simple)
  {
  int i = 10;
  srand(99);

  usleep(500000 + rand() % 200000);
  }


  BENCHMARK_F(ProjectionBase, Simple)
  {
  int i = myVal;
  srand(99);

  usleep(500000 + rand() % 200000);
  }*/
