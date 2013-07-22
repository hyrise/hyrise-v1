// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_AGGEGATEFUNCTIONS_H_
#define SRC_LIB_ACCESS_AGGEGATEFUNCTIONS_H_

#include <vector>

#include <storage/AbstractTable.h>
#include <storage/HashTable.h>
#include <storage/storage_types.h>

#include <json.h>

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
  virtual void processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow);

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
  virtual void processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow);

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
  virtual void processValuesForRows(const hyrise::storage::c_atable_ptr_t& t, pos_list_t *rows, hyrise::storage::atable_ptr_t& target, size_t targetRow) ;

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
