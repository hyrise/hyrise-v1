#ifndef SRC_LIB_ACCESS_ABSTRACTEXPRESSION_H_
#define SRC_LIB_ACCESS_ABSTRACTEXPRESSION_H_

#include <vector>
#include "helper/types.h"
#include <stdexcept>

namespace hyrise {
namespace access {

/// Abstract expression interface
class AbstractExpression {
 public:
  virtual ~AbstractExpression() {}
  virtual void walk(const std::vector<storage::c_atable_ptr_t>& l) = 0;
  virtual void match(storage::pos_list_t* pl, const size_t start, const size_t stop) = 0;
  virtual std::unique_ptr<AbstractExpression> clone() {
    throw std::runtime_error("Cannot clone base class; implement in derived");
  }

  virtual storage::pos_list_t matchAll(storage::pos_list_t&) {
    throw std::runtime_error(std::string() + typeid(*this).name() + " did not implement matchAll(pos)");
  }
  virtual storage::pos_list_t matchAll(size_t start, size_t stop) {
    throw std::runtime_error(std::string() + typeid(*this).name() + " did not implement matchAll(pos)");
  }
};
}
}

#endif
