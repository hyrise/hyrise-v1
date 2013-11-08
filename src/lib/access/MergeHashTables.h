// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MERGEHASHTABLES_H_
#define SRC_LIB_ACCESS_MERGEHASHTABLES_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

/// PlanOp that merges several hashtables. Primarily used tp execute HashBuild in parallel
class MergeHashTables : public PlanOperation {
public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setKey(const std::string &key);
  const std::string getKey() const;

private:
    std::string _key;
};

}
}

#endif  // SRC_LIB_ACCESS_MERGEHASHTABLES_H_
