// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_LAYOUTER_H_
#define SRC_LIB_ACCESS_LAYOUTER_H_

#include "access/system/PlanOperation.h"
#include "layouter/incremental.h"

namespace hyrise {
namespace access {

class LayoutSingleTable : public PlanOperation {
public:
  enum layouter_type {
    BaseLayouter,
    CandidateLayouter,
    DivideAndConquerLayouter
  };

  typedef struct {
    std::vector<unsigned> positions;
    int weight;
    double selectivity;
  } BaseQuery;

  LayoutSingleTable();
  virtual ~LayoutSingleTable();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void addFieldName(const std::string &n);
  void addQuery(const BaseQuery &q);
  /// Use the candidate layouter instead of calculating cost for all
  /// possible layouts
  void setLayouter(const layouter_type c);
  void setNumRows(const size_t n);
  /// Sets the number of layouts that should be returned from this
  /// plan op. All additional layouts are added as new rows to the
  /// result table.
  void setMaxResults(const size_t n);

private:
  typedef std::vector<std::string> names_list_t;
  typedef std::vector<unsigned> size_list_t;
  typedef std::vector<BaseQuery> query_list_t;

  layouter::Query *parseQuery(const BaseQuery &q);

  names_list_t _names;
  size_list_t _atts;
  query_list_t _queries;
  size_t _numRows;
  layouter_type _layouter;
  size_t _maxResults;
};

}
}

#endif  // SRC_LIB_ACCESS_LAYOUTER_H_
