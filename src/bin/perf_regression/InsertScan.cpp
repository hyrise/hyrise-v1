// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>

#include <string>
#include <chrono>

#include "io/EmptyLoader.h"
#include "io/TransactionManager.h"
#include "io/CSVLoader.h"
#include "io/shortcuts.h"
#include "access/InsertScan.h"
#include "access/tx/Commit.h"

namespace hyrise {
namespace access {

class InsertScanBase : public ::testing::Benchmark {

 protected:
  storage::atable_ptr_t data;
  storage::atable_ptr_t t;
  tx::TXContext ctx;
  access::InsertScan is;
  access::Commit c;

 public:
  void BenchmarkSetUp() {
    data = io::Loader::shortcuts::load("test/test10k_12.tbl");
    ctx = tx::TransactionManager::getInstance().buildContext();

    io::EmptyInput input;
    io::CSVHeader header("test/test10k_12.tbl");
    t = io::Loader::load(io::Loader::params().setInput(input).setHeader(header));
    is.setEvent("NO_PAPI");
    is.setTXContext(ctx);
    is.addInput(t);
    is.setInputData(data);
    c.setEvent("NO_PAPI");
    c.setTXContext(ctx);
    c.addInput(t);
  }

  InsertScanBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};

BENCHMARK_F(InsertScanBase, insert_single_tx_no_commit) { is.execute(); }

BENCHMARK_F(InsertScanBase, insert_single_tx_commit) {
  is.execute();
  c.execute();
}


template <typename F>
void bench(F func, std::size_t threads_sz = 10) {
  std::vector<std::thread> threads;
  for (std::size_t i = 0; i < threads_sz; i++) {
    threads.emplace_back(func);
  }
  for (auto& t : threads) {
    t.join();
  }
}

typedef struct {
  std::function<std::pair<storage::atable_ptr_t, storage::atable_ptr_t>()> base;
  std::size_t threads;
} params;

class InsertTest : public ::testing::TestWithParam<params> {
 public:
  storage::atable_ptr_t data, tbl;
  std::size_t threads;
  void SetUp() {
    auto param = GetParam();
    std::tie(data, tbl) = param.base();
    threads = param.threads;
  }
};

std::vector<params> generateParams() {
  std::vector<params> results;
  for (std::size_t t : {1, 2, 4, 8, 16, 32}) {
    for (std::size_t opt = 0; opt < 3; opt++) {
      auto func = [=]() {
        auto data = io::Loader::shortcuts::load("test/test10k_12.tbl");
        storage::atable_ptr_t tbl;
        // slightly hack-ish: opt distinguishes different insertion options
        if (opt == 0) {
          tbl = io::Loader::shortcuts::load("test/test10k_12.tbl",
                                            io::Loader::params().setReturnsMutableVerticalTable(true));
        } else {
          tbl = data->copy_structure_modifiable();
          tbl->resize(opt);
          for (auto col = 0u; col < 20; ++col) {
            for (auto row = 0u; row < opt; ++row) {
              tbl->setValue<hyrise_int_t>(col, row, row + col);
            }
          }
        }
        return std::make_pair(data, tbl);
      };
      results.push_back({func, t});
    }
  }
  return results;
}


TEST_P(InsertTest, concurrent_writes_single_insert) {
  auto d_before = data->size();
  auto insert_rows = tbl->size();
  const auto runs = 1000000 / insert_rows / threads;
  auto before = std::chrono::high_resolution_clock::now();
  auto l = [&]() {
    for (std::size_t d = 0; d < runs; d++) {
      auto ctx = tx::TransactionManager::getInstance().buildContext();
      InsertScan is;
      is.setEvent("NO_PAPI");
      is.setTXContext(ctx);
      is.addInput(data);
      is.setInputData(tbl);
      Commit c;
      c.setEvent("NO_PAPI");
      c.setTXContext(ctx);
      is.execute();
      c.execute();
    }
  };
  bench(l, threads);

  auto after = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
  this->RecordProperty("threads", threads);
  this->RecordProperty("inserts/commit", tbl->size());
  this->RecordProperty("rows/ms", (data->size() - d_before) / ms);
}

INSTANTIATE_TEST_CASE_P(InsertTestInst, InsertTest, ::testing::ValuesIn(generateParams()));
}
}
