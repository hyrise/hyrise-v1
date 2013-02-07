#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <io/loaders.h>
#include <storage/AbstractTable.h>

class LoaderBenchmark : public ::testing::Benchmark {
 protected:
  std::shared_ptr<AbstractTable> t;
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
  t = Loader::load(
      Loader::params()
      .setHeader(CSVHeader("benchmark_data/customer.tbl"))
      .setInput(CSVInput("benchmark_data/customer.tbl"))
                   );
}

BENCHMARK_F(LoaderBenchmark, LoadOrder) {
  t = Loader::load(
      Loader::params()
      .setHeader(CSVHeader("benchmark_data/order.tbl"))
      .setInput(CSVInput("benchmark_data/order.tbl"))
                   );
}

BENCHMARK_F(LoaderBenchmark, LoadHistory) {
  t = Loader::load(
      Loader::params()
      .setHeader(CSVHeader("benchmark_data/history.tbl"))
      .setInput(CSVInput("benchmark_data/history.tbl"))
                   );
}
