// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_STRINGLOADER_H_
#define SRC_LIB_IO_STRINGLOADER_H_

#include "io/AbstractLoader.h"

class StringHeader : public AbstractHeader {
 public:
  explicit StringHeader(std::string header) :
      _header(header) {
  }
  compound_metadata_list *load(const Loader::params &args);
  StringHeader *clone() const;
 private:
  std::string _header;
};

#endif  // SRC_LIB_IO_STRINGLOADER_H_
