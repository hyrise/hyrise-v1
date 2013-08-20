// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifdef WITH_MYSQL

#include "io/MySQLLoader.h"

#include <iostream>
#include <limits>
#include <map>

#include <boost/assign.hpp>
#include <boost/algorithm/string/erase.hpp>

#include <mysql/mysql.h>


#include "storage/ColumnMetadata.h"
#include "storage/MutableVerticalTable.h"
#include "storage/storage_types.h"
#include "storage/storage_types.h"
#include "storage/TableBuilder.h"

param_member_impl(MySQLInput::params, std::string, Host);
param_member_impl(MySQLInput::params, std::string, Port);
param_member_impl(MySQLInput::params, std::string, User);
param_member_impl(MySQLInput::params, std::string, Password);
param_member_impl(MySQLInput::params, std::string, Schema);
param_member_impl(MySQLInput::params, std::string, Table);
param_member_impl(MySQLInput::params, uint64_t, Limit);

typedef struct {
  std::string name;
  std::string type;
} column_meta;

typedef std::vector<column_meta> table_meta;


using namespace boost::assign;

/*!
  Writing a conversion function pitfalls:

  * sql::ResultSet* column access is 1-based, thus all accesses should use col+1
  */

typedef void (*conversion_func)(MYSQL_ROW *, size_t, size_t, AbstractTable *);

void var_to_string(MYSQL_ROW *r, size_t col, size_t row, AbstractTable *t) {
  if ((*r)[col] == 0)
    t->setValue<hyrise_string_t>(col, row, "");
  else
    t->setValue<hyrise_string_t>(col, row, std::string((const char *)(*r)[col]));
}

void bigint_to_integer(MYSQL_ROW *r, size_t col, size_t row, AbstractTable *t) {
  hyrise_int_t data = (*r)[col] == 0 ? 0 : atoll((*r)[col]);
  t->setValue<hyrise_int_t>(col, row, data);
}

void int_to_integer(MYSQL_ROW *r, size_t col, size_t row, AbstractTable *t) {
  hyrise_int_t data = (*r)[col] == 0 ? 0 : atoi((*r)[col]);
  t->setValue<hyrise_int_t>(col, row, data);
}

void double_to_float(MYSQL_ROW *r, size_t col, size_t row, AbstractTable *t) {
  hyrise_float_t data = (*r)[col] == 0 ? 0 : atof((*r)[col]);
  t->setValue<hyrise_float_t>(col, row, data);
}

void date_to_int(MYSQL_ROW *r, size_t col, size_t row, AbstractTable *t) {
  // Replace dashes with nothing and we have the int
  if ((*r)[col] == 0)
    t->setValue<hyrise_int_t>(col, row, 1);// std::stoll(s));
  else {
    std::string s((const char *)(*r)[col]);
    boost::algorithm::erase_all(s, "-");
    t->setValue<hyrise_int_t>(col, row, std::stoll(s));
  }
}


/*!
  This map is used for the conversion of MYSQL types to HYRISE types.

  MYSQLTYPESTRING->[HYRISETYPESTRING, CONVERSION_FUNCTION_PTR]
*/
std::map<std::string, std::pair<hyrise::types::type_t, conversion_func> > translations = map_list_of
    ("varchar",  make_pair(hyrise::types::string_t, var_to_string))
    ("char",     make_pair(hyrise::types::string_t, var_to_string))
    ("bigint",   make_pair(hyrise::types::integer_t, bigint_to_integer))
    ("int",      make_pair(hyrise::types::integer_t, int_to_integer))
    ("smallint", make_pair(hyrise::types::integer_t, int_to_integer))
    ("double",   make_pair(hyrise::types::float_t, double_to_float))
    ("date",     make_pair(hyrise::types::integer_t, date_to_int))
    ("time",     make_pair(hyrise::types::string_t, var_to_string))
    ("datetime", make_pair(hyrise::types::string_t, var_to_string));

std::shared_ptr<AbstractTable> MySQLInput::load(
  std::shared_ptr<AbstractTable> intable,
  const compound_metadata_list *meta,
  const Loader::params &args) {


  /** Initialize the MySQL connection with the parameters given to the
   * loader, in the second step we will then extract the meta data
   * from the table and load the table with the correct size
   */
  MYSQL *conn = mysql_init(NULL);
  if (!mysql_real_connect(conn, _parameters.getHost().c_str(),
                          _parameters.getUser().c_str(),
                          _parameters.getPassword().c_str(), "information_schema", 0, NULL, 0)) {
    //throw std::runtime_error("mysql connection failed " + strerror(errno));
  }

  std::string q1 = ("SELECT COLUMN_NAME, DATA_TYPE FROM COLUMNS WHERE TABLE_NAME='" + _parameters.getTable() + "' AND TABLE_SCHEMA='" + _parameters.getSchema() + "'");
  mysql_query(conn, q1.c_str());
  MYSQL_RES *res = mysql_use_result(conn);
  MYSQL_ROW row;


  // Build the table
  hyrise::storage::TableBuilder::param_list list;
  std::vector<std::string> typeList;
  while ((row = mysql_fetch_row(res)) != NULL) {
    typeList.push_back(std::string(row[1]));
    list.append().set_type(translations[typeList.back()].first).set_name(std::string(row[0]));
  }
  mysql_free_result(res);
  auto t = hyrise::storage::TableBuilder::build(list, args.getCompressed());


  mysql_query(conn, ("USE " + _parameters.getSchema()).c_str());

  uint64_t totalSize;
  mysql_query(conn, ("SELECT count(*) as count FROM " + _parameters.getTable()).c_str());
  res = mysql_use_result(conn);
  row = mysql_fetch_row(res);
  totalSize = atol(row[0]);
  mysql_free_result(res);

  totalSize = _parameters.getLimit() == 0 || totalSize < _parameters.getLimit() ? totalSize : _parameters.getLimit();

  t->resize(totalSize);
  mysql_query(conn, ("SELECT * FROM " + _parameters.getTable() + " LIMIT " + std::to_string(totalSize)).c_str());


  /** Iterate over all rows. It is important that we use
   * mysql_use_con() here so that we don't store the complete result
   * in HYRISE but rather do it recordwise.
   */
  res = mysql_use_result(conn);
  size_t rownum = 0;
  size_t column = 0;
  while ((row = mysql_fetch_row(res)) != NULL) {
    for (size_t i = 0; i < typeList.size(); ++i) {
      translations[typeList[i]].second(&row, column, rownum, t.get());
      ++column;
    }
    column = 0;
    ++rownum;
  }
  mysql_free_result(res);
  mysql_close(conn);
  return t;
}

MySQLInput *MySQLInput::clone() const {
  return new MySQLInput(*this);
}

#endif
