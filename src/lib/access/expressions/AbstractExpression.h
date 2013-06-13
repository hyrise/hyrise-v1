#ifndef SRC_LIB_ACCESS_ABSTRACTEXPRESSION_H_
#define SRC_LIB_ACCESS_ABSTRACTEXPRESSION_H_

#include <vector>
#include "helper/types.h"

namespace hyrise { namespace access {

/// Abstract expression interface
class AbstractExpression {
 public:
  virtual ~AbstractExpression() {}
  virtual void walk(const std::vector<storage::c_atable_ptr_t> &l) = 0;
  virtual storage::pos_list_t* match(const size_t start, const size_t stop) = 0;
};

}}

#endif
