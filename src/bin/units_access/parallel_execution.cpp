// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/ParallelizablePlanOperation.h"
#include "access/NoOp.h"
#include "testing/test.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

typedef struct {
  std::uint64_t start;
  std::uint64_t stop;
} d_result_t;
typedef struct {
  int rows;
  int part;
  int count;
} d_params_t;
typedef struct {
  d_result_t expected_result;
  d_params_t params;
  std::string info;
} test_param;


class ParallelExecutionTest : public testing::TestWithParam<test_param> {};

std::vector<test_param> tests = {{{0u, 1000u}, {2000, 0, 2}, "First partition"},
                                 {{1000u, 2000u}, {2000, 1, 2}, "Second partition"},
                                 {{3000u, 4000u}, {5000, 3, 5}, "Third partition of five"},
                                 {{0u, 1000u}, {5002, 0, 5}, "First partition of 5002 records split in five"},
                                 {{1000u, 2000u}, {5002, 1, 5}, "Second partititon of 5002 records"},
                                 {{4000u, 5002u}, {5002, 4, 5}, "Last should cover extra records"}};

// test data distribution method applied to PlanOp input
TEST_P(ParallelExecutionTest, data_distribution) {
  const auto& p = GetParam();
  const auto& expected = p.expected_result;
  const auto& params = p.params;

  auto result = ParallelizablePlanOperation::distribute(params.rows, params.part, params.count);
  EXPECT_EQ(std::make_pair(expected.start, expected.stop), result) << p.info;
}

INSTANTIATE_TEST_CASE_P(ParallelExecutionTests, ParallelExecutionTest, ::testing::ValuesIn(tests));


class DistributeCoveringTest : public ::testing::TestWithParam<std::tr1::tuple<unsigned int, unsigned int>> {};

TEST_P(DistributeCoveringTest, base) {
  const auto& p = GetParam();
  const auto count = std::tr1::get<0>(p);
  const auto rows = std::tr1::get<1>(p);
  std::uint64_t previous_last = 0;
  for (std::size_t part = 0; part < count; ++part) {
    auto result = ParallelizablePlanOperation::distribute(rows, part, count);
    EXPECT_EQ(previous_last, result.first) << "Must overlap with previous result";
    previous_last = result.second;
  }
  EXPECT_EQ(rows, previous_last) << "Last partition must cover all records";
}

INSTANTIATE_TEST_CASE_P(DistributeCoveringTests,
                        DistributeCoveringTest,
                        ::testing::Combine(::testing::Values(1u, 2u, 3u, 11u, 14u),
                                           ::testing::Values(0u, 1u, 10u, 13u, 1000u, 1001u, 1002u, 3333u)));
}
}
