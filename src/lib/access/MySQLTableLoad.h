#ifndef SRC_LIB_ACCESS_MYSQLTABLELOAD_H_
#define SRC_LIB_ACCESS_MYSQLTABLELOAD_H_

#ifdef WITH_MYSQL

#include <access/PlanOperation.h>

class MySQLTableLoad : public _PlanOperation {
public:
  MySQLTableLoad();
  virtual ~MySQLTableLoad();

  void executePlanOperation();

  void setDatabaseName(std::string databaseName);
  void setTableName(std::string tablename);

  /*! Parses:
    \verbatim
    {
    "type": "MySQLTableLoad",
    "database": "cbtr",
    "table": "KNA1"
    }
    \endverbatim
  */
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  static bool is_registered;

  static std::string name();
  const std::string vname();

  void setLoadLimit(uint64_t l) {
    _load_limit = l;
  }
private:
  std::string _table_name;
  std::string _database_name;
  uint64_t _load_limit;

};

#endif

#endif  // SRC_LIB_ACCESS_MYSQLTABLELOAD_H_
