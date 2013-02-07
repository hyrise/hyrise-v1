#ifndef SRC_LIB_ACCESS_GETTABLE_H
#define SRC_LIB_ACCESS_GETTABLE_H

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// Provides the ability to get an already loaded table
/// from StorageManager for use in subsequent steps
class GetTable : public _PlanOperation {
public:
  GetTable(const std::string& name);
  virtual ~GetTable();

  void executePlanOperation();
  const std::string vname();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

private:
  const std::string _name;
};

}
}

#endif /* SRC_LIB_ACCESS_GETTABLE_H */
