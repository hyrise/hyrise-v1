// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_ABSTRACTATTRIBUTEVECTOR_H_
#define SRC_LIB_STORAGE_ABSTRACTATTRIBUTEVECTOR_H_

#include <cstddef>

class AbstractAttributeVector {
 public:

  virtual ~AbstractAttributeVector();

  virtual void *data() = 0;
  virtual void setNumRows(size_t s) = 0;

};

#endif  // SRC_LIB_STORAGE_ABSTRACTATTRIBUTEVECTOR_H_
