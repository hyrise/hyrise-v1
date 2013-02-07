// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_AGGEGATEFUNCTIONS_H_
#define SRC_LIB_ACCESS_AGGEGATEFUNCTIONS_H_

#include <storage/AbstractTable.h>
#include <storage/HashTable.h>
#include <storage/storage_types.h>
#include <storage/meta_storage.h>
#include <json.h>
#include <vector>




/* helper construct to avoid excessive use
 * of switch case in executePlanOperation
 * uses templated type_switch in src/lib/storage/meta_storage.h
 * and calls the the correct templated operator implemented
 * below
 */
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

    target->setValue<R>(targetColumn, targetRow, result);
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

  if (rows != NULL) {
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

  target->setValue<float>(targetColumn, targetRow, ((float)sum / count));
}

template<>
void average_aggregate_functor::operator()<std::string>();
}
}

struct AggregateFunctions {
  enum type {
    SUM,
    COUNT,
    AVG
  };
};

typedef std::map<std::string, AggregateFunctions::type> aggregateFunctionMap_t;

aggregateFunctionMap_t getAggregateFunctionMap();

class AggregateFun;

AggregateFun *parseAggregateFunction(const Json::Value &value);

/*
  This is the base function for all aggregate functions. It defers the
  type handling down to the process Method and only returns
*/
class AggregateFun {
 public:
  field_t  _field;
  field_name_t _field_name;

  AggregateFun() {}
  explicit AggregateFun(field_t f): _field(f) {}
  explicit AggregateFun(field_name_t field_name): _field_name(field_name) {}

  virtual void walk(const AbstractTable &table);
  field_t getField() {
    return _field;
  };
  field_name_t getFieldName() {
    return _field_name;
  };
  virtual ~AggregateFun() {};
  virtual void processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, 
    pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow) = 0;
  virtual DataType getType() const = 0;
  virtual std::string columnName(const std::string &oldName) = 0;
};

class SumAggregateFun: public AggregateFun {

  DataType _datatype;

 public:
  explicit SumAggregateFun(field_t field): AggregateFun(field) { }
  explicit SumAggregateFun(field_name_t field): AggregateFun(field) { }

  virtual ~SumAggregateFun() {};

  /*!
   * executes the function only considering
   * the given rows in map_range_t rows
   * if rows == nullptr the functor is executed
   * on all rows of the input table
   */
  virtual void processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow) {

    hyrise::storage::sum_aggregate_functor fun(t, target, rows, _field, columnName(t->nameOfColumn(_field)), targetRow);
    hyrise::storage::type_switch<hyrise_basic_types> ts;
    ts(_datatype, fun);
  }

  virtual DataType getType() const {
    return _datatype;
  }

  virtual void walk(const AbstractTable &table) {
    AggregateFun::walk(table);
    _datatype = table.typeOfColumn(_field);
    if (_datatype == StringType) {
      throw std::runtime_error("Cannot calculate sum for column of StringType");
    }
  }

  virtual std::string columnName(const std::string &oldName) {
    return "SUM(" + oldName + ")";
  }

  static AggregateFun *parse(const Json::Value &);
};

class CountAggregateFun: public AggregateFun {
 public:

  explicit CountAggregateFun(field_t field): AggregateFun(field) { }
  explicit CountAggregateFun(field_name_t field): AggregateFun(field) { }
  virtual ~CountAggregateFun() {};

  /*!
   * counts the values in a column in rows
   * mapped to map_range_t rows
   * if map_range_t rows == nullptr all values
   * are considered for counting.
   */
  virtual void processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow) {
    size_t count;

    if (rows != nullptr) {
      count = rows->size();
    } else {
      count = t->size();
    }

    target->setValue<hyrise_int_t>(columnName(t->nameOfColumn(_field)), targetRow, count);
  }

  virtual DataType getType() const {
    return IntegerType;
  }

  virtual std::string columnName(const std::string &oldName) {
    return "COUNT(" + oldName + ")";
  }

  static AggregateFun *parse(const Json::Value &);
};

class AverageAggregateFun: public AggregateFun {
 public:
  AverageAggregateFun(field_t field): AggregateFun(field) { }
  AverageAggregateFun(field_name_t field): AggregateFun(field) { }
  DataType _datatype;
  virtual ~AverageAggregateFun() {};

  /*!
   * executes the function only considering
   * the given rows in map_range_t rows
   * if rows == NULL the functor is executed
   * on all rows of the input table
   */
  virtual void processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow) {
    hyrise::storage::average_aggregate_functor fun(t, target, rows, _field, columnName(t->nameOfColumn(_field)), targetRow);
    hyrise::storage::type_switch<hyrise_basic_types> ts;
    ts(_datatype, fun);
  }

  virtual DataType getType() const {
    return FloatType;
  }

  virtual void walk(const AbstractTable &table) {
    AggregateFun::walk(table);
    _datatype = table.typeOfColumn(_field);
    if (_datatype == StringType) {
      throw std::runtime_error("Cannot calculate average for column of StringType");
    }
  }

  virtual std::string columnName(const std::string &oldName) {
    return "AVG(" + oldName + ")";
  }

  static AggregateFun *parse(const Json::Value &);
};

#endif // AGGEGATEFUNCTIONS
