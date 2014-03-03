// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "access/system/PlanOperation.h"
#include "storage/AbstractResource.h"
#include "access/PipelineObserver.h"
#include "access/PipelineEmitter.h"
#include "helper/types.h"

namespace hyrise {
namespace access {

/*
 * This operator is used to split blocking input into several chunks
 * without performing any actual operation on it. It is only used at
 * beginnings of pipelines in order to yield higher degrees of parallelism
 * within a pipeline.
 */
class PipelineStream : public PlanOperation, public PipelineEmitter<PipelineStream> {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
};
}
}
