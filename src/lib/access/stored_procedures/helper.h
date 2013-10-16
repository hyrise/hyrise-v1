// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_STOREDPROCEDURES_HELPER_H_
#define SRC_LIB_ACCESS_STOREDPROCEDURES_HELPER_H_

#include <map>
#include <sstream>


namespace hyrise { namespace access {

typedef std::map<std::string, std::string> result_map_t;
typedef std::map<std::string, std::string> file_map_t;

const std::string tpccTableDir = "test/tpcc";
const std::string tpccDelimiter = ",";
const file_map_t tpccHeaderLocation = {{"DISTRICT"  , tpccTableDir + "/district_header.tbl"},
                                       {"WAREHOUSE" , tpccTableDir + "/warehouse_header.tbl"},
                                       {"CUSTOMER"  , tpccTableDir + "/customer_header.tbl"},
                                       {"HISTORY"   , tpccTableDir + "/history_header.tbl"},
                                       {"ORDERS"    , tpccTableDir + "/orders_header.tbl"},
                                       {"NEW_ORDER" , tpccTableDir + "/new_order_header.tbl"},
                                       {"ORDER_LINE", tpccTableDir + "/order_line.tbl"}};
const file_map_t tpccDataLocation = {{"DISTRICT"  , tpccTableDir + "/district.tbl"},
                                     {"WAREHOUSE" , tpccTableDir + "/warehouse.tbl"},
                                     {"CUSTOMER"  , tpccTableDir + "/customer.tbl"},
                                     {"HISTORY"   , tpccTableDir + "/history.tbl"},
                                     {"ORDERS"    , tpccTableDir + "/orders.tbl"},
                                     {"NEW_ORDER" , tpccTableDir + "/new_order.tbl"},
                                     {"ORDER_LINE", tpccTableDir + "/order_line.tbl"}};


template <typename T>
std::string toString(const T& t) {
  std::ostringstream os;
  os << t;
  return os.str();
}

}} // namespace hyrise::access

#endif // SRC_LIB_ACCESS_STOREDPROCEDURES_HELPER_H_

