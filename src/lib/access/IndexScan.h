// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INDEX_SCAN
#define SRC_LIB_ACCESS_INDEX_SCAN

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class AbstractIndexValue {
};

template<typename T>
class IndexValue : public AbstractIndexValue {
public:
  typedef T value_type;
  value_type value;
};

/// Scan an existing index for the result. Currently only EQ predicates
/// allowed for the index.
class IndexScan : public PlanOperation {
public:
  virtual ~IndexScan();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setIndexName(const std::string &name);
  template<typename T>
  void setValue(const T value) {
    auto val = new IndexValue<T>();
    val->value = value;
    _value = static_cast<AbstractIndexValue*>(val);
  }

private:
  std::string _indexName;
  AbstractIndexValue *_value;
};


class MergeIndexScan : public PlanOperation {
public:
  virtual ~MergeIndexScan();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
};

}
}

#endif // SRC_LIB_ACCESS_INDEX_SCAN
