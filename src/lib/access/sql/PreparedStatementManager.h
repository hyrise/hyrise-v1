// Copyright (c) 2015 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SQL_PREPAREDSTATEMENTMANAGER_H_
#define SRC_LIB_ACCESS_SQL_PREPAREDSTATEMENTMANAGER_H_

#include "access/sql/parser/PrepareStatement.h"
#include <string>
#include <map>

namespace hyrise {
namespace access {
namespace sql {

class PreparedStatementManager {
public:
  // Retrieve singleton instance
  static PreparedStatementManager& getInstance();

  bool hasStatement(std::string name);
  void addStatement(std::string name, hsql::PrepareStatement* stmt);
  hsql::PrepareStatement* getStatement(std::string name);
  void deleteStatement(std::string name);


 private:
 	std::map<std::string, hsql::PrepareStatement*> _data_map;

  PreparedStatementManager() = default;
  PreparedStatementManager(const PreparedStatementManager&) = delete;
  PreparedStatementManager& operator=(const PreparedStatementManager&) = delete;
};





} // namespace sql
} // namespace access
} // namespace hyrise
#endif