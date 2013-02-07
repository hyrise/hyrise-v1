#include "TestDataGenerator.h"
#include <helper/types.h>
#include <storage/TableGenerator.h>
#include <storage/TableMerger.h>
#include <storage/LogarithmicMergeStrategy.h>
#include <storage/SequentialHeapMerger.h>
#include <vector>



void TestDataGenerator::intel_simple_01() {
  size_t main_size = 100;
  size_t delta_size = 50;

  TableGenerator g;

  std::vector<hyrise::storage::atable_ptr_t> tables = g.distinct_cols(11, main_size, delta_size);
  std::vector<hyrise::storage::c_atable_ptr_t> input;
  for(const auto& t : tables)
    input.push_back(t);

  std::cout << "writing main..." << std::endl;
  tables[0]->write("main.tbl");

  std::cout << "writing delta..." << std::endl;
  tables[1]->write("delta.tbl");

  TableMerger merger(new LogarithmicMergeStrategy(0), new SequentialHeapMerger());

  std::cout << "merging..." << std::endl;
  const auto& merged = merger.merge(input);

  std::cout << "writing merged table..." << std::endl;
  merged[0]->write("merged.tbl");
}
