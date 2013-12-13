// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AggregateFunctions.h"
#include <storage/meta_storage.h>
#include "json.h"

namespace hyrise { namespace storage {

struct aggregate_functor { 
  typedef void value_type;
  
  const c_atable_ptr_t& input;
  atable_ptr_t& target;
  pos_list_t *rows;
  field_t sourceField;
  std::string targetColumn;
  size_t targetRow;
  
  aggregate_functor(const c_atable_ptr_t& i,
                    atable_ptr_t& t,
                    pos_list_t *forRows,
                    field_t sourceF,
                    std::string column,
                    size_t toRow): input(i), target(t), rows(forRows), sourceField(sourceF), targetColumn(column), targetRow(toRow) {}
};

struct sum_aggregate_functor : aggregate_functor {
  sum_aggregate_functor(const c_atable_ptr_t& i,
                        atable_ptr_t& t,
                        pos_list_t *forRows,
                        field_t sourceF,
                        std::string column,
                        size_t toRow): aggregate_functor(i, t, forRows, sourceF, column, toRow) {}

  template <typename R>
  value_type operator()();
};

template <typename R>
void sum_aggregate_functor::operator()() {
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

template<>
void sum_aggregate_functor::operator()<std::string>() {
  throw std::runtime_error("Cannot calculate sum for column of StringType");
}

struct average_aggregate_functor : aggregate_functor {
  average_aggregate_functor(const c_atable_ptr_t& i,
                            atable_ptr_t& t,
                            pos_list_t *forRows,
                            field_t sourceF,
                            std::string column,
                            size_t toRow): aggregate_functor(i, t, forRows, sourceF, column, toRow) {}

  template <typename R>
  value_type operator()();
};

template <typename R>
void average_aggregate_functor::operator()() {
  R sum = 0;
  size_t count = 0;

  if (rows != nullptr) {
    for (const auto& currentRow: *rows) {
      sum += input->getValue<R>(sourceField, currentRow);
    }
    count = rows->size();
  } else {
    const size_t input_size = input->size();

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

struct min_aggregate_functor : aggregate_functor {
  min_aggregate_functor(const c_atable_ptr_t& i,
                            atable_ptr_t& t,
                            pos_list_t *forRows,
                            field_t sourceF,
                            std::string column,
                            size_t toRow): aggregate_functor(i, t, forRows, sourceF, column, toRow) {}

  template <typename R>
  value_type operator()() {
    R min;

    if (rows != nullptr) {
      min = input->getValue<R>(sourceField, rows->front());
      for (const auto& currentRow: *rows) {
        R cur = input->getValue<R>(sourceField, currentRow);
	if (cur < min)
	  min = cur;
      }
    }
    else {
      min = input->getValue<R>(sourceField, 0);
      const size_t input_size = input->size();

      for (size_t currentRow = 0; currentRow < input_size; ++currentRow) {
        R cur = input->getValue<R>(sourceField, currentRow);
	if (cur < min)
	  min = cur;
      }
    }
    target->setValue<R>(target->numberOfColumn(targetColumn), targetRow, min);
  }
};

struct max_aggregate_functor : aggregate_functor{
  max_aggregate_functor(const c_atable_ptr_t& i,
                            atable_ptr_t& t,
                            pos_list_t *forRows,
                            field_t sourceF,
                            std::string column,
                            size_t toRow): aggregate_functor(i, t, forRows, sourceF, column, toRow) {}

  template <typename R>
  value_type operator()() {
    R max;

    if (rows != nullptr) {
      max = input->getValue<R>(sourceField, rows->front());
      for (const auto& currentRow: *rows) {
        const R cur = input->getValue<R>(sourceField, currentRow);
	if (cur > max)
	  max = cur;
      }
    }
    else {
      max = input->getValue<R>(sourceField, 0);
      const size_t input_size = input->size();

      for (size_t currentRow = 0; currentRow < input_size; ++currentRow) {
        const R cur = input->getValue<R>(sourceField, currentRow);
	if (cur > max)
	  max = cur;
      }
    }
    target->setValue<R>(target->numberOfColumn(targetColumn), targetRow, max);
  }
};

} // namespace storage

namespace access {

aggregateFunctionMap_t getAggregateFunctionMap() {
  aggregateFunctionMap_t d;
  d["SUM"] = AggregateFunctions::SUM;
  d["COUNT"] = AggregateFunctions::COUNT;
  d["AVG"] = AggregateFunctions::AVG;
  d["MIN"] = AggregateFunctions::MIN;
  d["MAX"] = AggregateFunctions::MAX;
  return d;
}

AggregateFun *parseAggregateFunction(const Json::Value &data) {
  int ftype = -1;
  if (data["type"].isNumeric()) {
    ftype = data["type"].asUInt();
  } else if (data["type"].isString()) {
    ftype = getAggregateFunctionMap()[data["type"].asString()];
  }
  AggregateFun *aggregate;
  switch (ftype) {
    case AggregateFunctions::SUM:
      aggregate = SumAggregateFun::parse(data);
      break;
    case AggregateFunctions::COUNT:
      aggregate = CountAggregateFun::parse(data);
      break;
    case AggregateFunctions::AVG:
      aggregate = AverageAggregateFun::parse(data);
      break;
    case AggregateFunctions::MIN:
      aggregate = MinAggregateFun::parse(data);
      break;
    case AggregateFunctions::MAX:
      aggregate = MaxAggregateFun::parse(data);
      break;
    default:
      throw std::runtime_error("Aggregation function not supported in GroupByScan");
  }
  if (data["as"].isString())
    aggregate->columnName(data["as"].asString());
  return aggregate;
}

void AggregateFun::walk(const storage::AbstractTable &table) {
  if (_field_name.size() > 0) { //either _field_name is set
    _field = table.numberOfColumn(_field_name);
  } else { //or _field
    _field_name = table.nameOfColumn(_field);
  }

  if (_new_field_name.size() == 0) {
     columnName(defaultColumnName(_field_name));
  }
}


void SumAggregateFun::processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows,
                                           storage::atable_ptr_t& target, size_t targetRow) {
  storage::sum_aggregate_functor fun(t, target, rows, _field, columnName(), targetRow);
  storage::type_switch<hyrise_basic_types> ts;
  ts(_dataType, fun);
}

AggregateFun *SumAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new SumAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new SumAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
}


void CountAggregateFun::processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows,
                                             storage::atable_ptr_t& target, size_t targetRow) {
  size_t count;

  if (isDistinct())
    count = countRowsDistinct(t, rows);
  else
    count = countRows(t, rows);

  target->setValue<hyrise_int_t>(target->numberOfColumn(columnName()), targetRow, count);
}

size_t CountAggregateFun::countRows(const storage::c_atable_ptr_t& t, pos_list_t *rows) {
  if (rows != nullptr)
    return rows->size();
  return t->size();
}

namespace {
  struct row_comparison_functor {
    typedef bool value_type;

    const storage::c_atable_ptr_t& table;
    size_t col;
    pos_t row1, row2;

    row_comparison_functor(const storage::c_atable_ptr_t& t, size_t col, pos_t row1, pos_t row2):
      table(t), col(col), row1(row1), row2(row2) {}

    template <typename R>
    value_type operator()() {
      return table->getValue<R>(col, row1) == table->getValue<R>(col, row2);
    }
  };

  bool rowsEqual(const storage::c_atable_ptr_t& t, size_t col, pos_t r1, pos_t r2) {
    row_comparison_functor fun(t, col, r1, r2);
    storage::type_switch<hyrise_basic_types> ts;
    return ts(t->typeOfColumn(col), fun);
  }

  bool equalRowIn(const storage::c_atable_ptr_t&t, size_t col, const pos_list_t &rows, pos_t row) { 
    for (const auto& currentRow: rows) {
      if (rowsEqual(t, col, row, currentRow))
        return true;
    }
    return false;
  }
} // namespace

size_t CountAggregateFun::countRowsDistinct(const storage::c_atable_ptr_t& t, pos_list_t *rows) {
  pos_list_t distinctRows;

  if (rows == nullptr) {
    for (pos_t currentRow = 0; currentRow < t->size(); ++currentRow) {
       if (!equalRowIn(t, _field, distinctRows, currentRow))
         distinctRows.push_back(currentRow);
    }
    return distinctRows.size();
  }
  for (const auto& currentRow: *rows) {
    if (!equalRowIn(t, _field, distinctRows, currentRow))
      distinctRows.push_back(currentRow);
  }
  
  return distinctRows.size();
}

AggregateFun *CountAggregateFun::parse(const Json::Value &f) {
  CountAggregateFun* aggregate;
  
  if (f["field"].isNumeric()) aggregate = new CountAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) aggregate =  new CountAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
  
  
  aggregate->setDistinct(f["distinct"].isBool() && f["distinct"].asBool());

  return aggregate;
}


void AverageAggregateFun::processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows,
                                               storage::atable_ptr_t& target, size_t targetRow) { 
    storage::average_aggregate_functor fun(t, target, rows, _field, columnName(), targetRow);
    storage::type_switch<hyrise_basic_types> ts;
    ts(_dataType, fun);
}

AggregateFun *AverageAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new AverageAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new AverageAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
}


void MinAggregateFun::processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows,
                                           storage::atable_ptr_t& target, size_t targetRow) { 
    storage::min_aggregate_functor fun(t, target, rows, _field, columnName(), targetRow);
    storage::type_switch<hyrise_basic_types> ts;
    ts(_dataType, fun);
}

AggregateFun *MinAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new MinAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new MinAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
}


void MaxAggregateFun::processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows,
                                           storage::atable_ptr_t& target, size_t targetRow) { 
    storage::max_aggregate_functor fun(t, target, rows, _field, columnName(), targetRow);
    storage::type_switch<hyrise_basic_types> ts;
    ts(_dataType, fun);
}

AggregateFun *MaxAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new MaxAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new MaxAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
}

} } // namespace hyrise::access

