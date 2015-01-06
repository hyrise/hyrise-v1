// Copyright (c) 2015 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "access/sql/PreparedStatementManager.h"

using namespace hsql;

namespace hyrise {
namespace access {
namespace sql {


PreparedStatementManager& PreparedStatementManager::getInstance() {
	static PreparedStatementManager psm;
	return psm;
}

bool PreparedStatementManager::hasStatement(std::string name) {
	return _data_map.count(name) == 1;
}

void PreparedStatementManager::addStatement(std::string name, PrepareStatement* stmt) {
	_data_map[name] = stmt;
}

PrepareStatement* PreparedStatementManager::getStatement(std::string name) {
	return _data_map[name];
}

void PreparedStatementManager::deleteStatement(std::string name) {
	delete getStatement(name);
	_data_map.erase(name);
}



} // namespace sql
} // namespace access
} // namespace hyrise