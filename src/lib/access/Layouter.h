#ifndef SRC_LIB_ACCESS_LAYOUTER_H_
#define SRC_LIB_ACCESS_LAYOUTER_H_

#include <vector>
#include <iostream>

#include <layouter.h>
#include <access/PlanOperation.h>
#include <json.h>

class LayoutSingleTable : public _PlanOperation {

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

 private:

  // internal types for schema and queries
  typedef std::vector<std::string> names_list_t;
  typedef std::vector<unsigned> size_list_t;
  typedef std::vector<BaseQuery> query_list_t;

  names_list_t _names;
  size_list_t _atts;
  query_list_t _queries;
  size_t _numRows;
  layouter_type _layouter;
  size_t _maxResults;

  layouter::Query *parseQuery(BaseQuery q);

 public:

  LayoutSingleTable(): _numRows(0), _layouter(CandidateLayouter), _maxResults(1) {
    
  }

  virtual ~LayoutSingleTable() {}

  void addFieldName(std::string n) {
    _names.push_back(n);

    // TODO suport different attribute sizes
    _atts.push_back(4);
  }

  void addQuery(BaseQuery q) {
    _queries.push_back(q);
  }

  /*
    Use the candidate layuouter instead of calculating cost for all
    possible layouts
  */
  void setLayouter(layouter_type c) {
    _layouter = c;
  }

  void setNumRows(size_t n) {
    _numRows = n;
  }

  /*
    Sets the number of layouts that should be returned from this
    plan op. All additional layouts are added as new rows to the
    result table.
  */
  void setMaxResults(size_t n) {
    _maxResults = n;
  }

  void executePlanOperation();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;
  static std::string name() {
    return "LayoutSingleTable";
  }
  const std::string vname() {
    return "LayoutSingleTable";
  }

};


#endif  // SRC_LIB_ACCESS_LAYOUTER_H_
