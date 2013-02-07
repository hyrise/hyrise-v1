#ifndef SRC_LIB_ACCESS_SETTABLE_H
#define SRC_LIB_ACCESS_SETTABLE_H

#include "access/PlanOperation.h"

/// Provides the ability to put a table into the StorageManager
class SetTable : public _PlanOperation
{
public:
  SetTable(const std::string& name);
  virtual ~SetTable();
  void executePlanOperation();
  const std::string vname();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
private:
  const std::string _name;
};


#endif /* SRC_LIB_ACCESS_SETTABLE_H */
