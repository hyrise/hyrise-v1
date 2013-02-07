#ifndef SRC_LIB_ACCESS_REPLACETABLE_H
#define SRC_LIB_ACCESS_REPLACETABLE_H

#include "access/PlanOperation.h"

/// Provides the ability to put a table into the StorageManager
class ReplaceTable : public _PlanOperation
{
public:
  ReplaceTable(const std::string& name);
  virtual ~ReplaceTable();
  void executePlanOperation();
  const std::string vname();
  static std::string name() { return "ReplaceTable"; }
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
private:
  const std::string _name;
};


#endif /* SRC_LIB_ACCESS_REPLACETABLE_H */
