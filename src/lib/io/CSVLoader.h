// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>

#include "io/AbstractLoader.h"
#include "io/GenericCSV.h"
#include "io/LoaderException.h"

namespace hyrise {
namespace io {

bool detectHeader(const std::string &filename);

class CSVLoaderError : public Loader::Error {
 public:
  explicit CSVLoaderError(const std::string &what): Loader::Error(what)
  {}
};

class CSVInput : public AbstractInput {
 public:
  class params {
#include "parameters.inc"
    param_member(csv::params, CSVParams);
    param_member(bool, Unsafe);
    params() : CSVParams(), Unsafe(false) {}
  };

  CSVInput(std::string filename, const params &parameters = params()) :
      _filename(filename),
      _parameters(parameters)
  {}

  std::shared_ptr<storage::AbstractTable> load(std::shared_ptr<storage::AbstractTable>, const storage::compound_metadata_list *, const Loader::params &args);

  CSVInput *clone() const;
 private:
  std::string _filename;
  params _parameters;
};

class CSVHeader : public AbstractHeader {
 public:
  class params {
#include "parameters.inc"
    param_member(csv::params, CSVParams);
    params() : CSVParams()
    {}
  };

  CSVHeader(std::string filename, const params &parameters = params()) : _filename(filename), _parameters(parameters) {
  }
  storage::compound_metadata_list *load(const Loader::params &args);
  CSVHeader *clone() const;
 private:
  std::string _filename;
  params _parameters;
};

} } // namepsace hyrise::io

