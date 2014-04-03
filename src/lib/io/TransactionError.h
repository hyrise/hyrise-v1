// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_TRANSACTIONERROR_H_
#define SRC_LIB_IO_TRANSACTIONERROR_H_

namespace hyrise {
namespace tx {

class transaction_error : public std::logic_error {
  // this should be in its own error category
  // logic_error is semantically incorrect

  using std::logic_error::logic_error;
};
}
}

#endif
