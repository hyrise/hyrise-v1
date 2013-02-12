// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <layouter.h>

#include <storage/AbstractDictionary.h>
#include <storage/OrderIndifferentDictionary.h>
#include <storage/Table.h>

#include "Layouter.h"

#include "QueryParser.h"

bool LayoutSingleTable::is_registered = QueryParser::registerPlanOperation<LayoutSingleTable>();

using namespace layouter;

std::shared_ptr<_PlanOperation> LayoutSingleTable::parse(Json::Value &data) {
  std::shared_ptr<LayoutSingleTable> s = std::make_shared<LayoutSingleTable>();

  s->setNumRows(data["num_rows"].asUInt());

  if (data.isMember("layouter")) {

    if (data["layouter"] == "BaseLayouter")
      s->setLayouter(BaseLayouter);
    else if (data["layouter"] == "CandidateLayouter")
      s->setLayouter(CandidateLayouter);
    else if (data["layouter"] == "DivideAndConquerLayouter")
      s->setLayouter(DivideAndConquerLayouter);
    else
      throw std::runtime_error("Layouter not available, chose different implementation");

  } else {
    s->setLayouter(BaseLayouter);
  }


  // Parse attributes
  if (data["attributes"].isArray()) {
    for (unsigned i = 0; i < data["attributes"].size(); ++i) {
      s->addFieldName(data["attributes"][i].asString());
    }
  } else {
    throw QueryParserException("Parse LayoutSingleTable failed: no attributes defined");
  }
  // Parse queries
  if (data["operators"].isArray()) {
    Json::Value ops = data["operators"];
    for (unsigned i = 0; i < ops.size(); ++i) {
      Json::Value q = ops[i];
      BaseQuery query;
      query.selectivity = q["selectivity"].asDouble();
      query.weight = q["weight"].asUInt64();

      Json::Value a = q["attributes"];
      for (unsigned j = 0; j < a.size(); ++j) {
        std::string name = a[j].asString();
        // find name in _names to get the position
        for (unsigned k = 0; k < s->_names.size(); ++k)
          if (name.compare(s->_names[k]) == 0)
            query.positions.push_back(k);
      }

      // Add query to layouter
      s->addQuery(query);
    }
  } else {
    throw QueryParserException("Parse LayoutSingleTable failed: no queries defined");
  }

  // TODO from JSON Parsing
  if (data.isMember("max_results"))
    s->setMaxResults(data["max_results"].asUInt());
  else
    s->setMaxResults(10);

  return s;
}

layouter::Query *LayoutSingleTable::parseQuery(BaseQuery q) {
  LayouterConfiguration::access_type_t t = q.selectivity == 1.0 ?
      LayouterConfiguration::access_type_fullprojection : LayouterConfiguration::access_type_outoforder;

  double sel = q.selectivity == 1.0 ? -1.0 : q.selectivity;

  layouter::Query *q1 = new layouter::Query(t, q.positions, sel, q.weight);

  return q1;
}

void LayoutSingleTable::executePlanOperation() {

  layouter::Schema s(_atts, _numRows, _names);

  std::vector<layouter::Query *> qs;
for (const auto & q: _queries) {
    Query *tmp = parseQuery(q);
    qs.push_back(tmp);

    s.add(tmp);
  }

  // Perform Layout Calculation
  layouter::BaseLayouter *bl;
  std::vector<layouter::Result> r;
  size_t size = 0;

  switch (_layouter) {
    case BaseLayouter:
      bl = new layouter::BaseLayouter();
      break;
    case DivideAndConquerLayouter:
      bl = new layouter::DivideAndConquerLayouter();
      break;
    case CandidateLayouter:
    default:
      bl = new layouter::CandidateLayouter();
      break;
  }

  bl->layout(s, HYRISE_COST);

  r = bl->getNBestResults(_maxResults);
  size = bl->count();



  hyrise::storage::atable_ptr_t result;
  metadata_list vc;
  vc.push_back(ColumnMetadata::metadataFromString("STRING", "content"));
  vc.push_back(ColumnMetadata::metadataFromString("INTEGER", "numResults"));
  vc.push_back(ColumnMetadata::metadataFromString("FLOAT", "totalCost"));
  vc.push_back(ColumnMetadata::metadataFromString("INTEGER", "numContainer"));
  vc.push_back(ColumnMetadata::metadataFromString("FLOAT", "columnCost"));
  vc.push_back(ColumnMetadata::metadataFromString("FLOAT", "rowCost"));


  std::vector<hyrise::storage::dict_ptr_t > vd;
  vd.push_back(DictionaryFactory<OrderIndifferentDictionary>::build(StringType));
  vd.push_back(DictionaryFactory<OrderIndifferentDictionary>::build(IntegerType));
  vd.push_back(DictionaryFactory<OrderIndifferentDictionary>::build(FloatType));
  vd.push_back(DictionaryFactory<OrderIndifferentDictionary>::build(IntegerType));
  vd.push_back(DictionaryFactory<OrderIndifferentDictionary>::build(FloatType));
  vd.push_back(DictionaryFactory<OrderIndifferentDictionary>::build(FloatType));

  // Allocate a new Table
  result = std::make_shared<Table<>>(&vc, &vd, _maxResults, false, 0, 64);

  result->resize(r.size());
  for (size_t i = 0; i < r.size(); ++i) {
    layouter::Result t = r[i];

    result->setValue<std::string>(0, i, t.output());
    result->setValue<hyrise_int_t>(1, i, size);
    result->setValue<float>(2, i, t.totalCost);
    result->setValue<hyrise_int_t>(3, i, t.cost.size());
    result->setValue<float>(4, i, bl->getColumnCost());
    result->setValue<float>(5, i, bl->getRowCost());
  }


  // Free the memory
  delete bl;

for (const auto & m: vc) {
    delete m;
  }

for (const auto & q: qs) {
    delete q;
  }

  addResult(result);
}


