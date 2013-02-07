#ifndef SRC_LIB_ACCESS_INTERSECTPOSITIONS_H_
#define SRC_LIB_ACCESS_INTERSECTPOSITIONS_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

/// Intersects positions from two incoming pointercalculators on the same table
class IntersectPositions: public _PlanOperation {
 public:

  /// Allowed parameters
  /// Parameters: none
  /// Query graph inputs:
  /// 1. store
  /// 2. positions to invalidate in main
  /// 3. positions to invalidate in delta
  /// Query graph output:
  /// - store with invalidated rows
  static std::shared_ptr<_PlanOperation> parse(Json::Value& data);
  void executePlanOperation();
  virtual const std::string vname();
};

}
}

#endif
