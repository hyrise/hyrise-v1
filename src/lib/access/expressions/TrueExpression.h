// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TRUEEXPRESSION_H_
#define SRC_LIB_ACCESS_TRUEEXPRESSION_H_

#include "access/expressions/pred_SimpleExpression.h"
#include "helper/types.h"
#include "storage/storage_types.h"

namespace hyrise { namespace access {

class TrueExpression : public SimpleExpression {
 public:
  bool operator()(const size_t row) {
    return true;
  }

  virtual pos_list_t* match(const size_t start, const size_t stop) {
    auto pl = new pos_list_t(stop - start);
    for (size_t row = start; row < stop; ++row)
      pl->push_back(row);
    return pl;
  }
  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t> &l) {}
};

} } // namespace hyrise::access

#endif //SRC_LIB_ACCESS_TRUEEXPRESSION_H_

