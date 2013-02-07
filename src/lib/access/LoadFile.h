// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_LOADFILE_H_
#define SRC_LIB_ACCESS_LOADFILE_H_

#include "access/PlanOperation.h"

class LoadFile : public _PlanOperation {
 public:
  explicit LoadFile(const std::string& filename);
  void executePlanOperation();
  const std::string vname();
  static std::shared_ptr<_PlanOperation> parse(Json::Value& data);
 private:
  const std::string _filename;
};

#endif /* SRC_LIB_ACCESS_LOADFILE_H_ */
