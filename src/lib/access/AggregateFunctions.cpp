// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AggregateFunctions.h"
#include <storage/meta_storage.h>
#include "json.h"

namespace hyrise {
namespace storage {

struct sum_aggregate_functor {
  typedef void value_type;

  const hyrise::storage::c_atable_ptr_t& input;
  hyrise::storage::atable_ptr_t& target;
  pos_list_t *rows;
  field_t sourceField;
  std::string targetColumn;
  size_t targetRow;

  sum_aggregate_functor(const hyrise::storage::c_atable_ptr_t& i,
                        hyrise::storage::atable_ptr_t& t,
                        pos_list_t *forRows,
                        field_t sourceF,
                        std::string column,
                        size_t toRow): input(i), target(t), rows(forRows), sourceField(sourceF), targetColumn(column), targetRow(toRow) {};

  template <typename R>
  void operator()() {
    R result = 0;

    if (rows != nullptr) {
      for (const auto& currentRow: *rows) {
        result += input->getValue<R>(sourceField, currentRow);
      }
    } else {
      size_t input_size = input->size();

      for (size_t i = 0; i < input_size; ++i) {
        result += input->getValue<R>(sourceField, i);
      }
    }

    target->setValue<R>(target->numberOfColumn(targetColumn), targetRow, result);
  }
};

struct average_aggregate_functor {
  typedef void value_type;

  const hyrise::storage::c_atable_ptr_t& input;
  hyrise::storage::atable_ptr_t& target;
  pos_list_t *rows;
  field_t sourceField;
  std::string targetColumn;
  size_t targetRow;

  average_aggregate_functor(const hyrise::storage::c_atable_ptr_t& i,
                            hyrise::storage::atable_ptr_t& t,
                            pos_list_t *forRows,
                            field_t sourceF,
                            std::string column,
                            size_t toRow): input(i), target(t), rows(forRows), sourceField(sourceF), targetColumn(column), targetRow(toRow) {};

  template <typename R>
  void operator()();
};

template<typename R>
void average_aggregate_functor::operator()() {
  R sum = 0;
  int count = 0;

  if (rows != nullptr) {
    for (const auto& currentRow: *rows) {
      sum += input->getValue<R>(sourceField, currentRow);
    }
    count = rows->size();
  } else {
    size_t input_size = input->size();

    for (size_t i = 0; i < input_size; ++i) {
      sum += input->getValue<R>(sourceField, i);
    }
    count = input->size();
  }
  target->setValue<float>(target->numberOfColumn(targetColumn), targetRow, ((float)sum / count));
}

template<>
void average_aggregate_functor::operator()<std::string>() {
  throw std::runtime_error("Cannot calculate average for column of StringType");
}
}
}

aggregateFunctionMap_t getAggregateFunctionMap() {
  aggregateFunctionMap_t d;
  d["SUM"] = AggregateFunctions::SUM;
  d["COUNT"] = AggregateFunctions::COUNT;
  d["AVG"] = AggregateFunctions::AVG;
  return d;
}

AggregateFun *parseAggregateFunction(const Json::Value &data) {
  int ftype = -1;
  if (data["type"].isNumeric()) {
    ftype = data["type"].asUInt();
  } else if (data["type"].isString()) {
    ftype = getAggregateFunctionMap()[data["type"].asString()];
  }
  switch (ftype) {
    case AggregateFunctions::SUM:
      return SumAggregateFun::parse(data);
    case AggregateFunctions::COUNT:
      return CountAggregateFun::parse(data);
    case AggregateFunctions::AVG:
      return AverageAggregateFun::parse(data);
    default:
      throw std::runtime_error("Aggregation function not supported in GroupByScan");
  }
}

void AggregateFun::walk(const AbstractTable &table) {
  if (_field_name.size() > 0) {
    _field = table.numberOfColumn(_field_name);
  }
}


void SumAggregateFun::processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow) {
  hyrise::storage::sum_aggregate_functor fun(t, target, rows, _field, columnName(t->nameOfColumn(_field)), targetRow);
  hyrise::storage::type_switch<hyrise_basic_types> ts;
  ts(_datatype, fun);
}


AggregateFun *SumAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new SumAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new SumAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
};

void CountAggregateFun::processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow) {
    size_t count;

    if (rows != nullptr) {
      count = rows->size();
    } else {
      count = t->size();
    }

    target->setValue<hyrise_int_t>(target->numberOfColumn(columnName(t->nameOfColumn(_field))), targetRow, count);
  }

AggregateFun *CountAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new CountAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new CountAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
};

void AverageAggregateFun::processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow) { 
    hyrise::storage::average_aggregate_functor fun(t, target, rows, _field, columnName(t->nameOfColumn(_field)), targetRow);
    hyrise::storage::type_switch<hyrise_basic_types> ts;
    ts(_datatype, fun);
}


AggregateFun *AverageAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new AverageAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new AverageAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
};
