// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INDEX_SCAN
#define SRC_LIB_ACCESS_INDEX_SCAN

#include <access/PlanOperation.h>

#include <string>

class AbstractIndexValue {
public:
  virtual ~AbstractIndexValue() {};
};

template<typename T>
class IndexValue : public AbstractIndexValue {

public:
  virtual ~IndexValue() {};

  typedef T value_type;
  value_type value;
};


/*
  Scan an existing index for the result. Currently only EQ predicates
  allowed for the index.
 */
class IndexScan : public _PlanOperation {

  // index name
  std::string _indexName;

  // value to compare with
  AbstractIndexValue *_value;

public:

  virtual ~IndexScan() {
    delete _value;
  }

  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  static bool is_registered;
  static std::string name() {
    return "IndexScan";
  }
  const std::string vname() {
    return "IndexScan";
  }

};


class MergeIndexScan : public _PlanOperation {

public:

  virtual ~MergeIndexScan(){}

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;
  static std::string name() {
    return "MergeIndexScan";
  }
  const std::string vname() {
    return "MergeIndexScan";
  }


};

#endif // SRC_LIB_ACCESS_INDEX_SCAN
