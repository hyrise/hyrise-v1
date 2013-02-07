#ifndef SRC_LIB_IO_CSVLOADER_H_
#define SRC_LIB_IO_CSVLOADER_H_

#include <memory>

#include "io/AbstractLoader.h"
#include "io/GenericCSV.h"
#include "io/LoaderException.h"





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

  CSVInput(std::string filename,
           const params &parameters = params()) :
      _filename(filename),
      _parameters(parameters)
  {}

  std::shared_ptr<AbstractTable> load(std::shared_ptr<AbstractTable>, const compound_metadata_list *, const Loader::params &args);

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
  compound_metadata_list *load(const Loader::params &args);
  CSVHeader *clone() const;
 private:
  std::string _filename;
  params _parameters;
};

#endif  // SRC_LIB_IO_CSVLOADER_H_
