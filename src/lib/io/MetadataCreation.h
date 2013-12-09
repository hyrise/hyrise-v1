// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "helper/types.h"
#include "io/LoaderException.h"
#include "storage/storage_types.h"

namespace hyrise {
namespace storage {
class AbstractTable;
} // namespace storage

namespace io {

class MetadataCreationError : public Loader::Error {
 public:
  explicit MetadataCreationError(const std::string &what) : Loader::Error(what)
  {}
};

/**
 * Transforms a nested std::vector<std::vector<std::string> > into metadata vector
 **/
storage::compound_metadata_list *createMetadata(const std::vector<std::vector<std::string> > &lines,
                                                storage::c_atable_ptr_t tab = nullptr);

} } // namespace hyrise::io

