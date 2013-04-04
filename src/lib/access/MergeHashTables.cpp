// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <access/MergeHashTables.h>
#include <storage/HashTable.h>

#include "QueryParser.h"

bool MergeHashTables::is_registered = QueryParser::registerPlanOperation<MergeHashTables>();

void MergeHashTables::executePlanOperation() {
  // get first HashTable and merge subsequent tables into HashTable
  if (_key == "groupby" || _key == "selfjoin" ) {
  	if (getInputHashTable(0)->getFieldCount() == 1)
  		addResultHash(std::make_shared<SingleAggregateHashTable>(input.getHashTables()));
  	else	
  		addResultHash(std::make_shared<AggregateHashTable>(input.getHashTables()));
  } else if (_key == "join") {
  	if (getInputHashTable(0)->getFieldCount() == 1)
  		addResultHash(std::make_shared<SingleJoinHashTable>(input.getHashTables()));
  	else
  		addResultHash(std::make_shared<JoinHashTable>(input.getHashTables()));
  } else {
    throw std::runtime_error("Type in Plan operation HashBuild not supported; key: " + _key);
  }
}
