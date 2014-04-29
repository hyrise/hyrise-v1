// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PIPELINEUNION_H_
#define SRC_LIB_ACCESS_PIPELINEUNION_H_

#include "access/system/PlanOperation.h"
#include "storage/AbstractResource.h"
#include "storage/PointerCalculator.h"

#include "access/PipelineObserver.h"
#include "helper/locking.h"
#include "helper/Synchronized.h"

namespace hyrise {
namespace access {

/*
 * This operator is used to terminate pipelines and merge all table chunks.
 */
class PipelineUnion : public PlanOperation, public PipelineObserver<PipelineUnion> {
 public:
  void executePlanOperation() override;

  // custom implementation just collecting chunks. Union happens on final executePlanOperation()
  virtual void notifyNewChunk(storage::c_aresource_ptr_t chunk) override;

 private:
  Synchronized<OperationData, std::mutex> _pipelineInput;

  storage::c_atable_ptr_t unionTables(std::vector<storage::c_atable_ptr_t> tables);
};
}
}

#endif  // SRC_LIB_ACCESS_PIPELINEUNION_H_
