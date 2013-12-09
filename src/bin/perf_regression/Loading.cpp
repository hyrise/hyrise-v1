// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <io/loaders.h>
#include <storage/AbstractTable.h>

namespace hyrise {
namespace access {

class LoaderBenchmark : public ::testing::Benchmark {
 protected:
  std::shared_ptr<storage::AbstractTable> t;
 public:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  LoaderBenchmark() {
    SetNumIterations(5);
    SetWarmUp(0);
  }
};

BENCHMARK_F(LoaderBenchmark, LoadCustomer) {
  t = io::Loader::load(
      io::Loader::params()
      .setHeader(io::CSVHeader("benchmark_data/customer.tbl"))
      .setInput(io::CSVInput("benchmark_data/customer.tbl"))
                   );
}

BENCHMARK_F(LoaderBenchmark, LoadOrder) {
  t = io::Loader::load(
      io::Loader::params()
      .setHeader(io::CSVHeader("benchmark_data/order.tbl"))
      .setInput(io::CSVInput("benchmark_data/order.tbl"))
                   );
}

BENCHMARK_F(LoaderBenchmark, LoadHistory) {
  t = io::Loader::load(
      io::Loader::params()
      .setHeader(io::CSVHeader("benchmark_data/history.tbl"))
      .setInput(io::CSVInput("benchmark_data/history.tbl"))
                   );
}

} } // namespace hyrise::access

