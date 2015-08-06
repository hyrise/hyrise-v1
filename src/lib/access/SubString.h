// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SUBSTRING_H_
#define SRC_LIB_ACCESS_SUBSTRING_H_

#include "access/system/ParallelizablePlanOperation.h"

namespace hyrise {
namespace access {

/// This class implements the SubString function as a hyrise operation.
/// It creates new columns containing all the substrings.
/// You can set the fields that should be affected, the start and length of the substing
/// and the names of the new columns.
/// Usage:
/// ........
/// "substring" :{
///     "type" : "SubString",
///     "fields" : ["C_FIRST", "C_LAST", "C_CITY"],
///     "strstart" : [1,2,1],
///     "strcount" : [4,3,3],
///     "as" : ["SUBSTR_FIRST", "SUBSTR_LAST", "SUBSTR_CITY"]
/// },
/// ........
/// "strstart" contains the start indicies of the substrings
/// "strcount" contains the length of the strings
/// "as" contains the names of the new columns
///
/// "strstart", "strcount" ans "as" should have as much elements as "fields"

class SubString : public ParallelizablePlanOperation {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

  void addStart(const int& start);
  void addCount(const int& count);
  void addColName(const std::string& colName);
  int getStart(int col) const;
  int getCount(int col) const;
  std::string getColName(int col) const;

  int startSize() const;
  int countSize() const;
  int nameSize() const;

 private:
  std::vector<int> _start;
  std::vector<int> _count;
  std::vector<std::string> _colName;
};
}
}
#endif  // SRC_LIB_ACCESS_SUBSTRING_H_
