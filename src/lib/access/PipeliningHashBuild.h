// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PIPELININGHASHBUILD_H_
#define SRC_LIB_ACCESS_PIPELININGHASHBUILD_H_

#include "access/system/PlanOperation.h"
#include "access/PipelineObserver.h"
#include "access/PipelineEmitter.h"

#include "helper/types.h"

namespace hyrise {
namespace access {

class PipeliningHashBuild : public PlanOperation,
                            public PipelineObserver<PipeliningHashBuild>,
                            public PipelineEmitter<PipeliningHashBuild> {

 public:
  void setKey(const std::string& key);
  const std::string getKey() const;

  virtual std::shared_ptr<PlanOperation> copy();

  // PlanOperation interface
  virtual void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

 private:
  std::string _key;
};
}
}
#endif  // SRC_LIB_ACCESS_PIPELININGHASHBUILD_H_
