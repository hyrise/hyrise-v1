#ifndef SRC_LIB_IO_METADATACREATION_H_
#define SRC_LIB_IO_METADATACREATION_H_

#include <memory>
#include <string>
#include <vector>

#include "helper/types.h"
#include "io/LoaderException.h"
#include "storage/storage_types.h"


class AbstractTable;

class MetadataCreationError : public Loader::Error {
 public:
  explicit MetadataCreationError(const std::string &what) : Loader::Error(what)
  {}
};

/**
 * Transforms a nested std::vector<std::vector<std::string> > into metadata vector
 **/
compound_metadata_list *createMetadata(const std::vector<std::vector<std::string> > &lines,
                                       hyrise::storage::c_atable_ptr_t tab = std::shared_ptr<const AbstractTable>());

#endif  // SRC_LIB_IO_METADATACREATION_H_
