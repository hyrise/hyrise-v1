// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <vector>
#include <random>
#include <cstdint>

#include "helper/types.h"
#include "storage/storage_types.h"

#include "access/TableScan.h"
#include "access/expressions/pred_EqualsExpression.h"

#include "io/VectorLoader.h"
#include "io/StringLoader.h"
#include "io/Loader.h"

#include "helper/make_unique.h"

#include "storage/AbstractTable.h"

#include "access/expressions/ExampleExpression.h"
//#include "gperftools/profiler.h"

namespace hyrise {
namespace access {

static const size_t BNUM_VALUES = 10000000;
static const size_t DISTINCT_VALUES = BNUM_VALUES/100;
class ScanBench : public ::testing::Benchmark {
 public:
  std::vector<std::uint64_t> _data;
  storage::atable_ptr_t _table;
  std::shared_ptr<TableScan> _tablescanNormal;
  std::shared_ptr<TableScan> _tablescanSpecial;
  std::uint64_t _value;
  std::vector<std::uint64_t> _dict;

  void init_data() {
    _data.reserve(BNUM_VALUES);
    std::mt19937 gen(0);
    std::uniform_int_distribution<std::uint64_t> dis(0, DISTINCT_VALUES);
    for (size_t i=0; i < BNUM_VALUES; ++i) {
      _data.push_back(dis(gen));
    }
    _dict.resize(DISTINCT_VALUES);
    for (size_t i=0; i < DISTINCT_VALUES; ++i) {
      _dict[i] = i;
    }
  }

  ScanBench() {
    SetNumIterations(10);
    SetWarmUp(2);

    init_data();
    auto p = io::Loader::params()
        .setInput(io::VectorInput({_data}))
        .setHeader(io::StringHeader("col1\nINTEGER\n0_R"));
    _table = io::Loader::load(p);
    _value = _data[_data.size()-1];
  }


  
  void BenchmarkSetUp() {
    _tablescanNormal = std::make_shared<TableScan>(make_unique<EqualsExpression<hyrise_int_t>>(0, 0, _value));
    _tablescanNormal->setEvent("NO_PAPI");
    _tablescanNormal->addInput(_table);

    _tablescanSpecial = std::make_shared<TableScan>(make_unique<ExampleExpression>(0, _value));
    _tablescanSpecial->setEvent("NO_PAPI");
    _tablescanSpecial->addInput(_table);
  }

  void BenchmarkTearDown() {
  }
};

pos_list_t scan(const std::vector<std::uint64_t>& data, const std::vector<std::uint64_t>& dict, const std::uint64_t value) {
  pos_list_t positions;
  std::uint64_t valueid = dict[value];
  for (size_t index=0, end=data.size(); index < end; ++index) {
    if (data[index] == valueid) {
      positions.push_back(index);
    }
  }
  return positions;
}

BENCHMARK_F(ScanBench, CMP_scanperformance_vector_seq) {
  scan(_data, _dict, _value).size();
}

BENCHMARK_F(ScanBench, CMP_scanperformance_hyriseDefault) {
  _tablescanNormal->execute();
}

BENCHMARK_F(ScanBench, CMP_scanperformance_hyriseSpecial) {
  _tablescanSpecial->execute();
}

} } // namespace hyrise::access

