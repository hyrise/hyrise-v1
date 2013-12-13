// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>

#include <storage/AbstractTable.h>
#include <storage/HashTable.h>
#include <storage/storage_types.h>

#include <json.h>

namespace hyrise {
namespace access {

struct AggregateFunctions {
  enum type {
    SUM,
    COUNT,
    AVG,
    MIN,
    MAX
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
  AggregateFun() {}
  explicit AggregateFun(field_t field) :
    _field(field) {}
  explicit AggregateFun(field_name_t field_name) :
    _field_name(field_name) {}

  virtual void walk(const storage::AbstractTable &table);
  field_t getField();// {
   // return _field;
  //}
  field_name_t getFieldName();// {
   // return _field_name;
  //}
  virtual ~AggregateFun() { }
  virtual void processValuesForRows(const storage::c_atable_ptr_t& t, 
    pos_list_t *rows, storage::atable_ptr_t& target, size_t targetRow) = 0;
  virtual DataType getType() const = 0;
  std::string columnName() const
  {
    return _new_field_name;
  }
  void columnName(const std::string &name) {
    _new_field_name = name;
  }
  virtual std::string defaultColumnName(const std::string &oldName) = 0;
  
 protected:
  field_t  _field;
  field_name_t _field_name;
  field_name_t _new_field_name;
};

class SumAggregateFun: public AggregateFun {
 protected:
  DataType _dataType;

 public:
  explicit SumAggregateFun(field_t field) : AggregateFun(field) { }
  explicit SumAggregateFun(field_name_t field) : AggregateFun(field) { }

  virtual ~SumAggregateFun() {};

  /*!
   * executes the function only considering
   * the given rows in map_range_t rows
   * if rows == nullptr the functor is executed
   * on all rows of the input table
   */
  virtual void processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows, storage::atable_ptr_t& target, size_t targetRow);

  virtual DataType getType() const {
    return _dataType;
  }

  virtual void walk(const storage::AbstractTable &table) {
    AggregateFun::walk(table);
    _dataType = table.typeOfColumn(_field);
    if (_dataType == StringType) {
      throw std::runtime_error("Cannot calculate sum for column of StringType");
    }
  }

  virtual std::string defaultColumnName(const std::string &oldName) {
    return "SUM(" + oldName + ")";
  }

  static AggregateFun *parse(const Json::Value &);
};

class CountAggregateFun: public AggregateFun {
 protected:
  bool _distinct;
 
 public:
  explicit CountAggregateFun(field_t field, bool distinct = false) : AggregateFun(field), _distinct(distinct) { }
  explicit CountAggregateFun(field_name_t field, bool distinct = false) : AggregateFun(field), _distinct(distinct) { }
  virtual ~CountAggregateFun() {};

  /*!
   * counts the values in a column in rows
   * mapped to map_range_t rows
   * if map_range_t rows == nullptr all values
   * are considered for counting.
   */
  virtual void processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows, storage::atable_ptr_t& target, size_t targetRow);
  size_t countRows(const storage::c_atable_ptr_t& t, pos_list_t *rows);
  size_t countRowsDistinct(const storage::c_atable_ptr_t& t, pos_list_t *rows);

  void setDistinct(bool distinct) {
    _distinct = distinct;
  }
  bool isDistinct() const {
    return _distinct;
  }

  virtual DataType getType() const {
    return IntegerType;
  }

  virtual std::string defaultColumnName(const std::string &oldName) {
    if (isDistinct())
      return "COUNT(DISTINCT " + oldName + ")";
    return "COUNT(" + oldName + ")";
  }

  static AggregateFun *parse(const Json::Value &);
};

class AverageAggregateFun: public AggregateFun {
 protected:
  DataType _dataType;

 public:
  AverageAggregateFun(field_t field) : AggregateFun(field) { }
  AverageAggregateFun(field_name_t field) : AggregateFun(field) { }
  virtual ~AverageAggregateFun() { }

  /*!
   * executes the function only considering
   * the given rows in map_range_t rows
   * if rows == nullptr the functor is executed
   * on all rows of the input table
   */
  virtual void processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows, storage::atable_ptr_t& target, size_t targetRow) ;

  virtual DataType getType() const {
    return FloatType;
  }

  virtual void walk(const storage::AbstractTable &table) {
    AggregateFun::walk(table);
    _dataType = table.typeOfColumn(_field);
    if (_dataType == StringType) {
      throw std::runtime_error("Cannot calculate average for column of StringType");
    }
  }

  virtual std::string defaultColumnName(const std::string &oldName) {
    return "AVG(" + oldName + ")";
  }

  static AggregateFun *parse(const Json::Value &);
};

class MinAggregateFun: public AggregateFun {
 protected:
  DataType _dataType;

 public:
  MinAggregateFun(field_t field) : AggregateFun(field) { }
  MinAggregateFun(field_name_t field) : AggregateFun(field) { }
  virtual ~MinAggregateFun() { }

  /*!
   * executes the function only considering
   * the given rows in map_range_t rows
   * if rows == nullptr the functor is executed
   * on all rows of the input table
   */
  virtual void processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows, storage::atable_ptr_t& target, size_t targetRow) ;

  virtual DataType getType() const {
    return _dataType;
  }
  
  virtual void walk(const storage::AbstractTable &table) {
    AggregateFun::walk(table);
    _dataType = table.typeOfColumn(_field);
  }

  virtual std::string defaultColumnName(const std::string &oldName) {
    return "MIN(" + oldName + ")";
  }

  static AggregateFun *parse(const Json::Value &);
};

class MaxAggregateFun: public AggregateFun {
 protected:
  DataType _dataType;

 public:
  MaxAggregateFun(field_t field) : AggregateFun(field) { }
  MaxAggregateFun(field_name_t field) : AggregateFun(field) { }
  virtual ~MaxAggregateFun() { }

  /*!
   * executes the function only considering
   * the given rows in map_range_t rows
   * if rows == nullptr the functor is executed
   * on all rows of the input table
   */
  virtual void processValuesForRows(const storage::c_atable_ptr_t& t, pos_list_t *rows, storage::atable_ptr_t& target, size_t targetRow) ;

  virtual DataType getType() const {
    return _dataType;
  }
  
  virtual void walk(const storage::AbstractTable &table) {
    AggregateFun::walk(table);
    _dataType = table.typeOfColumn(_field);
  }

  virtual std::string defaultColumnName(const std::string &oldName) {
    return "MAX(" + oldName + ")";
  }

  static AggregateFun *parse(const Json::Value &);
};

} } // namespace hyrise::access

