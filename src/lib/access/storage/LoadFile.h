// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_LOADFILE_H_
#define SRC_LIB_ACCESS_LOADFILE_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class LoadFile : public PlanOperation {
public:
  explicit LoadFile(const std::string &filename);
  virtual ~LoadFile();

  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();

private:
  const std::string _filename;
};

}
}

#endif /* SRC_LIB_ACCESS_LOADFILE_H_ */
