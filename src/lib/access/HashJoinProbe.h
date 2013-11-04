// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_HASHJOINPROBE_H_
#define SRC_LIB_ACCESS_HASHJOINPROBE_H_

#include "access/system/ParallelizablePlanOperation.h"
#include "helper/types.h"

namespace hyrise {
namespace access {

/// The HashJoinProbe operator performs the probe phase of a hash join to
/// produce the join result.
/// It takes the build table's AbstractHashTable and the probe table as input.
class HashJoinProbe : public ParallelizablePlanOperation {
public:
  HashJoinProbe();
  virtual ~HashJoinProbe();

  void setupPlanOperation();
  void executePlanOperation();
  /// {
  ///     "operators": {
  ///         "0": {
  ///             "type": "TableLoad",
  ///             "table": "table1",
  ///             "filename": "..."
  ///         },
  ///         "1": {
  ///             "type": "TableLoad",
  ///             "table": "table2",
  ///             "filename": "..."
  ///         },
  ///         "2": {
  ///             "type": "HashBuild",
  ///             "fields" : [1]
  ///         },
  ///         "3": {
  ///             "type": "HashJoinProbe",
  ///             "fields" : [0]
  ///         }
  ///     },
  ///     "edges": [["0", "2"], ["2", "3"], ["1", "3"]]
  /// }
  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);
  const std::string vname();
  void setBuildTable(const storage::c_atable_ptr_t &table);
  storage::c_atable_ptr_t getBuildTable() const;
  storage::c_atable_ptr_t getProbeTable() const;

private:
  /// Hashes input table on-the-fly and probes hashed value against input
  /// AbstractHashTable to write matching rows in given position lists.
  template<class HashTable>
  void fetchPositions(storage::pos_list_t *buildTablePosList,
                      storage::pos_list_t *probeTablePosList);
  /// Constructs resulting table from given build and probe tables' rows.
  storage::atable_ptr_t buildResultTable(storage::pos_list_t *buildTablePosList,
                                         storage::pos_list_t *probeTablePosList) const;
  storage::c_atable_ptr_t _buildTable;
  bool _selfjoin;
};

}
}

#endif  // SRC_LIB_ACCESS_HASHJOINPROBE_H_
