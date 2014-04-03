// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <string>

#include "helper/types.h"
#include "storage/storage_types.h"


namespace hyrise {

namespace storage {
class AbstractTable;
}  // namespace storage

namespace io {
class AbstractInput;
class AbstractHeader;


typedef struct {
  bool InsertOnly;
  tx::transaction_id_t transaction_id;
} InsertOnlyParam;

namespace Loader {
class params {
 private:
#include "parameters.inc"
  /// Data import
  param_ref_member(AbstractInput, Input);
  /// Header import
  param_ref_member(AbstractHeader, Header);
  /// Base path for imports
  param_member(std::string, BasePath);
  /// Name for the table to be created
  param_member(std::string, TableName);
  /// Currently no effect:
  param_member(bool, ModifiableMutableVerticalTable);
  /// Return result without wrapping in Store
  param_member(bool, ReturnsMutableVerticalTable);
  /// Use bitcompressed table
  param_member(bool, Compressed);
  /// Reference table used for type detection
  param_member(storage::c_atable_ptr_t, ReferenceTable);

  param_member(bool, DeltaDataStructure);

 public:
  params();
  ~params();
  params(const params& other);
  params& operator=(const params& other);
  params* clone() const;
};

std::shared_ptr<storage::AbstractTable> load(const params& args);
};
}
}  // namespace hyrise::io
