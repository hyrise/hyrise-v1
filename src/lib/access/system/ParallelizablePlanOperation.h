#ifndef SRC_LIB_ACCESS_PARALLELIZABLEOPERATION_H_
#define SRC_LIB_ACCESS_PARALLELIZABLEOPERATION_H_

#include "access/system/PlanOperation.h"

namespace hyrise {
namespace access {

class ParallelizablePlanOperation : public PlanOperation {
 public:
  /// Compute start and end for accessing partition `part` of `count`
  /// parts in `numberOfElements`
  static std::pair<std::uint64_t, std::uint64_t> distribute(std::uint64_t numberOfElements,
                                                            std::size_t part,
                                                            std::size_t count);

  /// If operator is supposed to be a parallel instance of an operator,
  /// separate input data based on instance enumeration.
  virtual void splitInput();
  virtual void refreshInput();

  void setPart(size_t part);
  void setCount(size_t count);

 protected:
  size_t _part = 0;
  size_t _count = 0;
};
}
}

#endif
