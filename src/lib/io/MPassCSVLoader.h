// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>

#include "AbstractLoader.h"
#include "LoaderException.h"
#include "GenericCSV.h"
#include "CSVLoader.h"

namespace hyrise {
namespace io {

class MPassCSVInput : public AbstractInput {
 public:
  class params {
#include "parameters.inc"
    param_member(bool, Unsafe);
    params() : Unsafe(false) {}
  };

  MPassCSVInput(std::string directory, const params &parameters = params()) :
      _directory(directory),
      _parameters(parameters)
  {}

  std::shared_ptr<storage::AbstractTable> load(std::shared_ptr<storage::AbstractTable>, const storage::compound_metadata_list *, const Loader::params &args);

  bool needs_store_wrap() {
    return false;
  }

  MPassCSVInput *clone() const;
 private:
  std::string _directory;
  params _parameters;
};

} } // namespace hyrise::io

