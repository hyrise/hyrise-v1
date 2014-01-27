// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>

#include <algorithm>
#include <access.h>
#include <storage.h>
#include <io.h>

#include <helper/types.h>
#include <helper/SparseVector.h>


#include "tbb/concurrent_vector.h"

class ConcurrentVectorTest : public ::testing::Benchmark {

protected:

  constexpr static unsigned num = 10000000;
  
  using tbb_type = tbb::concurrent_vector<hyrise::tx::transaction_id_t>;
  using hyrise_type = hyrise::helper::SparseVector<hyrise::tx::transaction_id_t>;

  tbb_type _data1;
  hyrise_type _data2;
  std::vector<size_t> _pos;

public: 

  void SetUp() {
    for(size_t i=0; i < num; ++i) {
      _pos.push_back(i);
    }
    std::random_shuffle(std::begin(_pos), std::end(_pos));
  }

  void BenchmarkSetUp() {
    _data1 = tbb_type(num, hyrise::tx::UNKNOWN);
    _data2 = hyrise_type(num, hyrise::tx::UNKNOWN);
    for(size_t i=0; i < num; ++i) {
      if (i % 1000 == 0) {
        _data1[i] = i;
        _data2[i] = i;
      }
    }
  }

};


BENCHMARK_F(ConcurrentVectorTest, simple_set_concurrent_vector) {
  for(size_t i=0; i < num; ++i) {
    if (i % 1000 == 0)
      _data1[i] = i;
  }
}

BENCHMARK_F(ConcurrentVectorTest, simple_set_concurrent_sparse) {
  for(size_t i=0; i < num; ++i) {
    if (i % 1000 == 0)
      _data2[i] = i;
  }
}

BENCHMARK_F(ConcurrentVectorTest, simple_get_concurrent_vector) {
  hyrise::tx::transaction_id_t sum = 0;
  for(size_t i=0; i < num; ++i) {
    sum += _data1[i];
  }
}

BENCHMARK_F(ConcurrentVectorTest, simple_get_concurrent_sparse) {
  hyrise::tx::transaction_id_t sum = 0;
  for(size_t i=0; i < num; ++i) {
    sum += _data2.get(i);
  }
}

BENCHMARK_F(ConcurrentVectorTest, simple_get_concurrent_vector_rand) {
  hyrise::tx::transaction_id_t sum = 0;
  for(size_t i=0; i < num; ++i) {
    sum += _data1[_pos[i]];
  }
}

BENCHMARK_F(ConcurrentVectorTest, simple_get_concurrent_sparse_rand) {
  hyrise::tx::transaction_id_t sum = 0;
  for(size_t i=0; i < num; ++i) {
    sum += _data2.get(_pos[i]);
  }
}
