// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MERGEHASHTABLES_H_
#define SRC_LIB_ACCESS_MERGEHASHTABLES_H_

#include <access/PlanOperation.h>

/**
 * PlanOp that merges several hashtables. Primarily used tp execute HashBuild in parallel
 */
class MergeHashTables : public _PlanOperation {

 public:

  MergeHashTables() : _PlanOperation() { 
  }

  virtual ~MergeHashTables() {
  }

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data) {
    auto instance = std::make_shared<MergeHashTables>();
    if (data.isMember("key")) {
      instance->setKey(data["key"].asString());
    }
    return instance;
  }

  static bool is_registered;

  static std::string name() {
    return "MergeHashTables";
  }

  const std::string vname() {
    return "MergeHashTables";
  }

  void setKey(const std::string &key){
    _key = key;
  };
  
  const std::string getKey() const{
    return _key;
  };

  private:
    std::string _key;
};
#endif  // SRC_LIB_ACCESS_MERGEHASHTABLES_H_