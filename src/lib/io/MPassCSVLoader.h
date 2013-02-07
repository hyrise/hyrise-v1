#ifndef SRC_LIB_IO_MPASSCSVLOADER_H_
#define SRC_LIB_IO_MPASSCSVLOADER_H_

#include <memory>

#include "AbstractLoader.h"
#include "LoaderException.h"
#include "GenericCSV.h"
#include "CSVLoader.h"




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

  std::shared_ptr<AbstractTable> load(std::shared_ptr<AbstractTable>, const compound_metadata_list *, const Loader::params &args);

  bool needs_store_wrap() {
    return false;
  }

  MPassCSVInput *clone() const;
 private:
  std::string _directory;
  params _parameters;
};


#endif  // SRC_LIB_IO_MPASSCSVLOADER_H_
