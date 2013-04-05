// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_HASHBUILD_H_
#define SRC_LIB_ACCESS_HASHBUILD_H_

#include "PlanOperation.h"

namespace hyrise {
namespace access {

class HashBuild : public _PlanOperation {
public:
  virtual ~HashBuild();

  void executePlanOperation();
  /// {
  ///     "operators": {
  ///         "0": {
  ///             "type": "TableLoad",
  ///             "table": "table",
  ///             "filename": "..."
  ///         },
  ///         "1": {
  ///             "type": "HashBuild",
  ///             "fields" : [1]
  ///         },
  ///     },
  ///         "edges": [["0", "1"]]
  /// }
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  const std::string vname();
  void setKey(const std::string &key);
  const std::string getKey() const;

private:
  std::string _key;
};

}
}

#endif  // SRC_LIB_ACCESS_HASHBUILD_H_
