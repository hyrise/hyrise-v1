// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PIPELININGHASHPROBE_H_
#define SRC_LIB_ACCESS_PIPELININGHASHPROBE_H_

#include "access/system/PlanOperation.h"
#include "helper/types.h"

#include "access/HashBuild.h"

#include "access/PipelineObserver.h"
#include "access/PipelineEmitter.h"

namespace hyrise {
namespace access {

/// Ttakes the build table's AbstractHashTable and the probe table as input.
class PipeliningHashProbe : public PlanOperation,
                            public PipelineObserver<PipeliningHashProbe>,
                            public PipelineEmitter<PipeliningHashProbe> {
 public:
  PipeliningHashProbe();
  void setupPlanOperation();
  void executePlanOperation();
  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);
  void setBuildTable(const storage::c_atable_ptr_t& table);
  storage::c_atable_ptr_t getBuildTable() const;
  storage::c_atable_ptr_t getProbeTable() const;

  virtual std::shared_ptr<PlanOperation> copy();

  virtual void addCustomDependencies(taskscheduler::task_ptr_t newChunkTask);

 private:
  /// Hashes input table on-the-fly and probes hashed value against input
  /// AbstractHashTable to write matching rows in given position lists.
  template <class HashTable>
  void fetchPositions();
  std::shared_ptr<PlanOperation> getHashBuildPredecessor() const { return getFirstPredecessorOf<HashBuild>(); }
  void emitChunk();
  /// Constructs resulting table from given build and probe tables' rows.
  storage::atable_ptr_t buildResultTable() const;
  void resetPosLists();

  storage::c_atable_ptr_t _buildTable;

  storage::pos_list_t* _buildTablePosList;
  storage::pos_list_t* _probeTablePosList;

  bool _selfjoin = false;
};
}
}

#endif  // SRC_LIB_ACCESS_PIPELININGHASHPROBE_H_
