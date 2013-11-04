// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_JSONTABLE_H_
#define SRC_LIB_ACCESS_JSONTABLE_H_

#include "access/system/PlanOperation.h"


namespace hyrise {
namespace access {


/**
* Create a new Table based on a Json Plan Operation
* 
* This class provides a simple to use interface to the functionality that is
* provided by the TableBuilder class. 
*
*/
class JsonTable : public PlanOperation {

	std::vector<std::string> _names;
	std::vector<std::string> _types;
	std::vector<unsigned> _groups;
  std::vector<std::vector<std::string>> _data;


  std::vector<std::string> _serialFields;
	bool _useStoreFlag;
  bool _mergeFlag;

public:

	JsonTable();

  void executePlanOperation();

  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

  // set the names to be used as attribute names in the table
  void setNames(const std::vector<std::string> names);

  // set the types
  void setTypes(const std::vector<std::string> types);

  // set the atribute grouping
  void setGroups(const std::vector<unsigned> groups);

  void setUseStore(const bool);
  void setMergeStore(const bool);

  void setData(const std::vector<std::vector<std::string>> data);

};

}
}

#endif // SRC_LIB_ACCESS_JSONTABLE_H_
