// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/IndexAwareTableScan.h"

#include <memory>

#include "access/system/BasicParser.h"
#include "access/json_converters.h"
#include "access/system/QueryParser.h"

#include "io/StorageManager.h"

#include "storage/InvertedIndex.h"
#include "storage/Store.h"
#include "storage/meta_storage.h"
#include "storage/PointerCalculator.h"

#include "helper/checked_cast.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<IndexAwareTableScan>("IndexAwareTableScan");
}

// IndexAwareTableScan::IndexAwareTableScan(std::unique_ptr<AbstractExpression> expr) 
//   : _ts(std::move(expr)) {
// }

IndexAwareTableScan::IndexAwareTableScan() { }

void IndexAwareTableScan::executePlanOperation() {

  size_t field = _field_definition[0];

  IndexScan _is;
  TableScan _ts(std::unique_ptr<EqualsExpression<hyrise_int_t> >(new EqualsExpression<hyrise_int_t>(0, field, _value)));
  
  // disable papi trace as nested operators breaks it
  _is.disablePapiTrace();
  _ts.disablePapiTrace();

  // Prepare Index Scan for the Main
  storage::c_atable_ptr_t t = input.getTables()[0];
  _is.addInput(t);
  _is.addField(field);
  _is.setValue<hyrise_int_t>(_value);
  _is.setIndexName(_index_name);

  // Preapare Table Scan for the Delta
  auto t_store = std::dynamic_pointer_cast<const storage::Store>(t);
  if(t_store) {
    _ts.addInput(t_store->getDeltaTable());
  } else {
    _ts.addInput(t);      
  }

  _is.execute();
  _ts.execute();

  auto pc1 = checked_pointer_cast<const PointerCalculator>(_is.getResultTable());
  auto pc2 = checked_pointer_cast<const PointerCalculator>(_ts.getResultTable());

  if(t_store) {
    pos_list_t pos = *pc2->getPositions();
    pos_t offset = t_store->getMainTables()[0]->size();

    std::for_each(pos.begin(), pos.end(), [&offset](pos_t& d) { d += offset;});

    std::shared_ptr<PointerCalculator> pc2_absolute(PointerCalculator::create(t));
    pc2_absolute->setPositions(pos);
    addResult(pc1->concatenate(pc2_absolute));
  } else {
    addResult(pc1->concatenate(pc2));
  }
}

std::shared_ptr<PlanOperation> IndexAwareTableScan::parse(Json::Value &data) {

  std::shared_ptr<IndexAwareTableScan> idx_scan = BasicParser<IndexAwareTableScan>::parse(data);

  if (data.isMember("index"))
    idx_scan->setIndexName(data["index"].asString());

  if (data.isMember("value"))
    idx_scan->setValue(data["value"].asInt());

  return idx_scan;
}

const std::string IndexAwareTableScan::vname() {
  return "IndexAwareTableScan";
}

void IndexAwareTableScan::setIndexName(const std::string &name) {
   _index_name = name;
}

void IndexAwareTableScan::setValue(const hyrise_int_t value) {
  _value = value;
}

}
}

