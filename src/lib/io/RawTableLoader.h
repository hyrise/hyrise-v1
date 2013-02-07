// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_RAWTABLELOADER_H_
#define SRC_LIB_IO_RAWTABLELOADER_H_

#include "io/AbstractLoader.h"
#include "io/GenericCSV.h"

namespace hyrise { namespace io {

/**
 * Loading the contents of a file in a uncompressed container
 * 
 * This is a slightly modified version of the CSVInput
 * class. Basically it provides less features and provides only the
 * bare possibilties to load the data uncompressed. 
 */
class RawTableLoader : public AbstractInput {

private:

  std::string _filename;
      
      
public:
      
  explicit RawTableLoader( std::string f) : _filename(f) {  
  }

  virtual ~RawTableLoader() {}

  std::shared_ptr<AbstractTable> load(std::shared_ptr<AbstractTable>,
                                      const compound_metadata_list *,
                                      const Loader::params &args);


  /// Never wrap a raw table with a store
  virtual bool needs_store_wrap() { 
    return false;
  }

  RawTableLoader* clone() const {
    return new RawTableLoader(*this);
  }


};

} }


#endif // SRC_LIB_IO_RAWTABLELOADER_H_
