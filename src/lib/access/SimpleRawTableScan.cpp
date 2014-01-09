// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SimpleRawTableScan.h"

#include "access/expressions/pred_buildExpression.h"

#include "storage/meta_storage.h"
#include "storage/PointerCalculator.h"
#include "storage/RawTable.h"
#include "storage/MutableVerticalTable.h"

namespace hyrise {
namespace storage {

/// Simple functor that copies all values from the src raw table to the
/// destination table. The only requirement is that the source table is
/// really an uncompressed table otherwise this will fail with a seg
/// fault.
struct copy_value_functor_raw_table {
  typedef void value_type;

  atable_ptr_t _dest;
  std::shared_ptr<const RawTable> _src;

  size_t _srcRow;
  size_t _destRow;
  size_t _field;

  copy_value_functor_raw_table(atable_ptr_t d,
                               std::shared_ptr<const RawTable> s) :
                               _dest(d),
                               _src(s) {
  }

  void setValues(const size_t destRow,
                 const size_t srcRow,
                 const size_t field ) {
    _destRow = destRow;
    _srcRow = srcRow; _field = field;
  }

  template<typename R>
  void operator()() {
    _dest->setValue<R>(_field, _destRow, _src->getValue<R>(_field, _srcRow));
  }
};

}
}

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<SimpleRawTableScan>("SimpleRawTableScan");
}

SimpleRawTableScan::SimpleRawTableScan(SimpleExpression* comp,
                                       const bool materializing) :
                                       PlanOperation(),
                                       _comparator(comp),
                                       _materializing(materializing) {
}

SimpleRawTableScan::~SimpleRawTableScan() {
}

void SimpleRawTableScan::setupPlanOperation() {
  _comparator->walk(input.getTables());
}

void SimpleRawTableScan::executePlanOperation() {
  auto table = std::dynamic_pointer_cast<const storage::RawTable>(input.getTable(0));
  if (!table)
    throw std::runtime_error("Input table is no uncompressed raw table");

  // Prepare a result that contains the result semi-compressed, and only row-wise
  storage::metadata_list meta(table->columnCount());
  for(size_t i=0; i<table->columnCount(); ++i)
    meta[i] = table->metadataAt(i);
  auto result = std::make_shared<storage::Table>(&meta,
                                                 nullptr,
                                                 0,  /* initial size */
                                                 false, /* sorted */
                                                 false /* compressed */);

  // Prepare the copy operator
  storage::copy_value_functor_raw_table fun(result, table);
  storage::type_switch<hyrise_basic_types> ts;
  auto positions = new storage::pos_list_t;

  size_t tabSize = table->size();
  for(size_t row=0; row < tabSize; ++row) {
    if ((*_comparator)(row)) {
      if (_materializing) {
        result->resize(result->size() + 1);
        for(size_t i=0; i < result->columnCount(); ++i) {
          fun.setValues(result->size() - 1, row, i);
          ts(table->typeOfColumn(i), fun);
        }
      }
      else {
        positions->push_back(row);
      }
    }
  }

  if (_materializing) {
    addResult(result);
  } else {
    addResult(storage::PointerCalculator::create(table, positions));
  }
}

std::shared_ptr<PlanOperation> SimpleRawTableScan::parse(const Json::Value &data) {
  if (!data.isMember("predicates")) {
    throw std::runtime_error("There is no reason for a Selection without predicates");
  }
  bool mat = data.isMember("materializing") ? data["materializing"].asBool() : true;
  // defaults to materializing
  return std::make_shared<SimpleRawTableScan>(buildExpression(data["predicates"]), mat);
}

const std::string SimpleRawTableScan::vname() {
  return "SimpleRawTableScan";
}

}
}
