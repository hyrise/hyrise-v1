// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PRED_SIMPLEEXPRESSION_H_
#define SRC_LIB_ACCESS_PRED_SIMPLEEXPRESSION_H_

#include "storage/storage_types.h"
#include "helper/types.h"

class SimpleExpression {
 public:
  SimpleExpression() { }
  virtual ~SimpleExpression() { }

  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t> &l) = 0;

  virtual pos_list_t* match(const size_t start, const size_t stop) {
    auto pl = new pos_list_t;
    for(size_t row=0; row < stop; ++row) {
      if (operator()(row)) {
        pl->push_back(row);
      }
    }
    return pl;
  }

  inline virtual bool operator()(size_t row) {
    throw std::runtime_error("Cannot call base class");
  }
};


#endif  // SRC_LIB_ACCESS_PRED_SIMPLEEXPRESSION_H_
