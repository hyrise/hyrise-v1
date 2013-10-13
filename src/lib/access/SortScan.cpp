// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SortScan.h"

#include <algorithm>

#include "access/system/QueryParser.h"

#include "storage/AbstractTable.h"
#include "storage/PointerCalculator.h"
#include "storage/Table.h"

namespace hyrise {
namespace access {

template <typename T>
struct ExtractValue {
  static inline T extractValue(const storage::c_atable_ptr_t &table,
                               const size_t &col,
                               const size_t &row) {
    return table->getValue<T>(col, row);
  }
};

template <typename T>
struct ExtractValueId {
  static inline ValueId extractValue(const storage::c_atable_ptr_t &table,
                                     const size_t &col,
                                     const size_t &row) {
    return table->getValueId(col, row);
  }
};

template <typename T, template<typename> class ExtractFunctor>
class ColumnSorter {
  typedef struct pair {
      T value;
      size_t row;
  } pair_t;

  const storage::c_atable_ptr_t &_t;
  field_t _f;
  bool _asc;

public:
  ColumnSorter(const storage::c_atable_ptr_t &t,
               const field_t f,
               const bool asc):
               _t(t),
               _f(f),
               _asc(asc) {
  }

  std::vector<pos_t>* sort() const {
    std::vector<pair_t> result;

    for (size_t row = 0; row < _t->size(); ++row) {
      result.push_back({ExtractFunctor<T>::extractValue(_t, _f, row), row});
    }

    auto asc_sort = [](const pair_t& left, const pair_t& right) { 
      return (left.value < right.value); 
    };

    auto desc_sort = [](const pair_t& left, const pair_t& right) { 
      return (left.value > right.value); 
    };


    std::stable_sort(result.begin(),
                     result.end(),
                     _asc ? asc_sort : desc_sort
                     );

    auto r = new std::vector<pos_t>;
    r->reserve(result.size());
    for (const pair_t& p: result)
      r->push_back(p.row);

    return r;
  }
};

namespace {
  auto _ = QueryParser::registerPlanOperation<SortScan>("SortScan");
}

SortScan::~SortScan() {
}

void SortScan::executePlanOperation() {
  const auto& table = input.getTable(0);
  // Sorted Position List
  std::vector<pos_t> *sorted_pos;

  // When table is not only a table but also using an ordered dictionary on sort field,
  // we can just sort by value_id
  // TODO: fix Table<> template
  auto base_table = std::dynamic_pointer_cast<const Table>(table);

  field_t sortField;
  if (_sort_field_name.size() != 0) // either a column name is specified
    sortField = table->numberOfColumn(_sort_field_name);
  else // or a column number
    sortField = _sort_field;

  if ((table->dictionaryAt(sortField)->isOrdered()) && (base_table)) {
    sorted_pos = ColumnSorter<ValueId, ExtractValueId>(table, sortField, asc).sort();
  } else {
    switch (table->metadataAt(sortField)->getType()) {
      case IntegerType:
        sorted_pos = ColumnSorter<hyrise_int_t, ExtractValue>(table, sortField, asc).sort();
        break;
      case FloatType:
        sorted_pos = ColumnSorter<hyrise_float_t, ExtractValue>(table, sortField, asc).sort();
        break;
      case StringType:
        sorted_pos = ColumnSorter<hyrise_string_t, ExtractValue>(table, sortField, asc).sort();
        break;
      default:
        throw std::runtime_error("Datatype not supported");
    }
  }

  storage::atable_ptr_t result;

  if (producesPositions) {
    result = PointerCalculator::create(table, sorted_pos);
  } else {
    result = table->copy_structure_modifiable(nullptr, true);
    size_t result_row = 0;
    for (const auto& p: *sorted_pos) {
      result->copyRowFrom(table, p, result_row++);
    }
    delete sorted_pos;
  }
  addResult(result);
}

std::shared_ptr<PlanOperation> SortScan::parse(Json::Value &data) {
  std::shared_ptr<SortScan> s = std::make_shared<SortScan>();
  if (data["fields"][0u].isNumeric()) {
    s->setSortField(data["fields"][0u].asUInt());
  }
  else if (data["fields"][0u].isString()) {
    s->setSortFieldName(data["fields"][0u].asString());
  }
  else
    throw std::runtime_error("Field for SortScan not specified correctly");

  if (data.isMember("asc"))
    s->asc = data["asc"].asBool();
  return s;
}

const std::string SortScan::vname() {
  return "SortScan";
}
void SortScan::setSortField(const unsigned s) {
  _sort_field = s;
}

void SortScan::setSortFieldName(const std::string& name) {
  _sort_field_name = name;
}

}
}
