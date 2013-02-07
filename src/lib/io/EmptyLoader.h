// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_EMPTYLOADER_H_
#define SRC_LIB_IO_EMPTYLOADER_H_

#include <memory>

#include "io/AbstractLoader.h"

class EmptyInput : public AbstractInput {
 public:
  EmptyInput() {
  }
  EmptyInput *clone() const;
  std::shared_ptr<AbstractTable> load(std::shared_ptr<AbstractTable>, const compound_metadata_list *, const Loader::params &args);
};

class EmptyHeader : public AbstractHeader {
 public:
  EmptyHeader() {
  }
  EmptyHeader *clone() const;
  compound_metadata_list *load(const Loader::params &args);
};


#endif  // SRC_LIB_IO_EMPTYLOADER_H_
