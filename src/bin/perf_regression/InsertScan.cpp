// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>
#include <sys/time.h>

#include <access.h>
#include <storage.h>
#include <io.h>
#include <io/EmptyLoader.h>
#include "io/TransactionManager.h"
#include <io/CSVLoader.h>
#include "io/shortcuts.h"

//JoinScan Benchmark similar to TPC-C Implementation of Stock-Level Transaction
//See TPC-C Reference Chapter A.5

class InsertScanBase : public ::testing::Benchmark {

 protected:

  hyrise::storage::atable_ptr_t data;
  hyrise::storage::atable_ptr_t t;
  hyrise::tx::TXContext ctx;
  hyrise::access::InsertScan is;
  hyrise::access::Commit c;
  unsigned long begin_time, total_time;
  uint _run, _warmUp, _numIterations;

 public:
  void BenchmarkSetUp() {
    data = Loader::shortcuts::load("test/test10k_12.tbl");
    ctx = hyrise::tx::TransactionManager::getInstance().buildContext();

    EmptyInput input;
    CSVHeader header("test/test10k_12.tbl");
    t = Loader::load(Loader::params().setInput(input).setHeader(header));

    is.setTXContext(ctx);
    is.addInput(t);
    is.setInputData(data);

    c.setTXContext(ctx);
    c.addInput(t);

    struct timeval tv;
    gettimeofday(&tv,NULL);
    begin_time = 1000000 * tv.tv_sec + tv.tv_usec;
  }

  void BenchmarkTearDown() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    if(_run == 0) total_time = 0;
    if(_run > _warmUp) total_time += (1000000 * tv.tv_sec + tv.tv_usec) - begin_time;
    if(_run == _warmUp + _numIterations - 1) printf("[      MSG ] RUNTIME_MEAN : %lu\n", total_time / _numIterations);
    _run++;
    _run %= (_warmUp + _numIterations);
  }

  InsertScanBase() {
    _warmUp = 2;
    _numIterations = 10;

    SetNumIterations(_numIterations);
    SetWarmUp(_warmUp);
    printf("[      MSG ] Running %u warm up runs and %u benchmark runs\n", _warmUp, _numIterations);
  }
};

BENCHMARK_F(InsertScanBase, insert_single_tx_no_commit)
  {
    is.execute();
  }

BENCHMARK_F(InsertScanBase, insert_single_tx_commit)
  {
    is.execute();
    c.execute();
  }
