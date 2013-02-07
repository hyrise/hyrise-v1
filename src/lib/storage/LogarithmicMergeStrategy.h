
#ifndef SRC_LIB_STORAGE_LOGARITHMICMERGESTRATEGY_H_
#define SRC_LIB_STORAGE_LOGARITHMICMERGESTRATEGY_H_

#include <helper/types.h>
#include <storage/AbstractMergeStrategy.h>
#include <storage/AbstractTable.h>

class LogarithmicMergeStrategy : public AbstractMergeStrategy {
 public:
  explicit LogarithmicMergeStrategy(size_t max_generation_count);
  ~LogarithmicMergeStrategy();

  virtual merge_tables determineTablesToMerge(std::vector<hyrise::storage::c_atable_ptr_t > &input_tables) const;
  virtual size_t calculateNewSize(const std::vector<hyrise::storage::c_atable_ptr_t > &input_tables) const;

  void setMaxGenerationCount(size_t max_generation_count);
  size_t getMaxGenerationCount();
  virtual AbstractMergeStrategy *copy();
 protected:
  size_t _max_generation_count;

};

#endif  // SRC_LIB_STORAGE_LOGARITHMICMERGESTRATEGY_H_
