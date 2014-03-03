// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PIPELINEMERGEHASHTABLES_H_
#define SRC_LIB_ACCESS_PIPELINEMERGEHASHTABLES_H_

#include "access/system/PlanOperation.h"
#include "storage/AbstractResource.h"
#include "storage/PointerCalculator.h"
#include "access/PipeliningHashBuild.h"
#include "access/system/OperationData.h"

#include "access/PipelineObserver.h"
#include "helper/Synchronized.h"

#include "taskscheduler/Task.h"

namespace hyrise {
namespace access {

class PipelineMergeHashTables : public PlanOperation, public PipelineObserver<PipelineMergeHashTables> {
 public:
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  virtual void notifyNewChunk(storage::c_aresource_ptr_t chunkTbl) override;

 private:
  std::string _hashKey;
  storage::ahashtable_ptr_t _hashResult;

  Synchronized<OperationData, std::mutex> _pipelineInput;

  storage::c_ahashtable_ptr_t mergeHashTables(std::vector<storage::c_ahashtable_ptr_t> tables);

  const std::string getHashKeyFromPredecessor() {
    auto phb = getFirstPredecessorOf<PipeliningHashBuild>();
    return phb->getKey();
  }
};
}
}

#endif  // SRC_LIB_ACCESS_PIPELINEMERGEHASHTABLES_H_
