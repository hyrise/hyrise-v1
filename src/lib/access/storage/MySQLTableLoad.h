// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MYSQLTABLELOAD_H_
#define SRC_LIB_ACCESS_MYSQLTABLELOAD_H_

#include <access/system/PlanOperation.h>

#ifdef WITH_MYSQL

namespace hyrise {
namespace access {

class MySQLTableLoad : public PlanOperation {
public:
  MySQLTableLoad();
  virtual ~MySQLTableLoad();

  void executePlanOperation();
  /// {
  /// "type": "MySQLTableLoad",
  /// "database": "cbtr",
  /// "table": "KNA1"
  /// }
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setDatabaseName(const std::string &databaseName);
  void setTableName(const std::string &tablename);
  void setLoadLimit(const uint64_t l);

private:
  std::string _table_name;
  std::string _database_name;
  uint64_t _load_limit;
};

}
}

#endif

#endif  // SRC_LIB_ACCESS_MYSQLTABLELOAD_H_
