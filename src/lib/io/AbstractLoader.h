// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "io/Loader.h"
#include "storage/storage_types.h"
#include "helper/types.h"

namespace hyrise {
namespace io {

class cloneable {
 public:
  virtual ~cloneable() {};
  virtual cloneable *clone() const = 0;
};

//! Interface for implementing a data source for loading data into a table
class AbstractInput : public cloneable {
 public:
  virtual ~AbstractInput() {};

  /*! Loads data.
    - Load data into a table
    - Creates and returns an AbstractTable*
    - Has to release intable in case it isn't used.

    @param intable The Loader::load interface creates an initial MutableVerticalTable, loader may choose to use it or not
    @param metadata The original metadata
    @param args Loader arguments that may influence loading
  */
  virtual std::shared_ptr<storage::AbstractTable> load(std::shared_ptr<storage::AbstractTable> intable,
                                                       const storage::compound_metadata_list *metadata,
                                                       const Loader::params &args) = 0;

  virtual AbstractInput *clone() const = 0;

  virtual bool needs_store_wrap() {
    return true;
  };
};


//! Interface for implementing something that yields a header structure suitable for table construction
class AbstractHeader : public cloneable {
 public:
  virtual ~AbstractHeader() {};
  /*! Construct metadata.
    @param args Loader arguments that may influence header creation
  */
  virtual storage::compound_metadata_list *load(const Loader::params &args) = 0;
  virtual AbstractHeader *clone() const = 0;
};

} } // namesapce hyrise::io

