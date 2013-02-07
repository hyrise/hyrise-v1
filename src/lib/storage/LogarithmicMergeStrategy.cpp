#include "storage/LogarithmicMergeStrategy.h"

LogarithmicMergeStrategy::LogarithmicMergeStrategy(size_t max_generation_count) :
    _max_generation_count(max_generation_count) {

}

LogarithmicMergeStrategy::~LogarithmicMergeStrategy() {

}

// calculate new size - insert only
size_t LogarithmicMergeStrategy::calculateNewSize(const std::vector<hyrise::storage::c_atable_ptr_t >& input_tables) const {
  size_t new_size = 0;

  for (size_t table = 0; table < input_tables.size(); table++) {
    new_size += input_tables[table]->size();
  }

  return new_size;
}

// all tables that should be merged are moved to tables_to_merge
// the rest stays in input_tables
merge_tables LogarithmicMergeStrategy::determineTablesToMerge(std::vector<hyrise::storage::c_atable_ptr_t > &input_tables) const {
  std::vector<hyrise::storage::c_atable_ptr_t > tables_to_merge, tables_not_to_merge;

  if (_max_generation_count == 0) {
    // this is the immediate merge strategy
    // we always merge all tables
    tables_to_merge = input_tables;
  } else {
    size_t generation_count = 0, carry = 0, merge_count = 0;

    // input_tables is sorted descending by generation
    // step through it backwards and add all tables which violate
    // _max_generation_count to tables_to_merge
    for (size_t i = input_tables.size(); i > 0; i--) {

      // calculate size of block with tables with same generation
      generation_count++;

      if (i == 1 || input_tables[i - 1]->generation() != input_tables[i - 2]->generation()) {
        // we are at the end of one block
        if (generation_count + carry > _max_generation_count) {
          // the block ist to big an needs to be merged
          merge_count += generation_count;

          if (generation_count > _max_generation_count) {
            carry++;
          }

          generation_count = 0;
        } else {
          break;
        }

        // is there a gap in generations?
        if (i > 1 && (input_tables[i - 1]->generation() + 1 < input_tables[i - 2]->generation())) {
          break;
        }
      }
    }

    // add tables that need to be merged
    tables_to_merge.reserve(merge_count);

    for (size_t i = 0; i < input_tables.size() - merge_count; i++) {
      tables_not_to_merge.push_back(input_tables[i]);
    }

    for (size_t i = input_tables.size() - merge_count; i < input_tables.size(); i++) {
      tables_to_merge.push_back(input_tables[i]);
    }
  }

  return _merge_tables(tables_to_merge, tables_not_to_merge);
}

void LogarithmicMergeStrategy::setMaxGenerationCount(size_t max_generation_count) {
  _max_generation_count = max_generation_count;
}

size_t LogarithmicMergeStrategy::getMaxGenerationCount() {
  return _max_generation_count;
}


AbstractMergeStrategy *LogarithmicMergeStrategy::copy() {
  return new LogarithmicMergeStrategy(_max_generation_count);
}

