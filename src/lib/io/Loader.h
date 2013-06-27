// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_IO_LOADER_H_
#define SRC_LIB_IO_LOADER_H_

#include <memory>
#include <string>

#include "helper/types.h"
#include "storage/storage_types.h"
#include "storage/ColumnProperties.h"


class AbstractTable;
class AbstractInput;
class AbstractHeader;
class AbstractTableFactory;

typedef struct {
  bool InsertOnly;
  hyrise::tx::transaction_id_t transaction_id;
} InsertOnlyParam;

namespace Loader {
class params {
private:
#include "parameters.inc"
  /// Data import
  param_ref_member(AbstractInput, Input);
  /// Header import
  param_ref_member(AbstractHeader, Header);
  /// Factory for tables
  param_member(AbstractTableFactory *, Factory);
  /// Base path for imports
  param_member(std::string, BasePath);
  /// Should input contain validity fields?
  param_member(InsertOnlyParam, InsertOnly);
  /// Currently no effect:
  param_member(bool, ModifiableMutableVerticalTable);
  /// Return result without wrapping in Store
  param_member(bool, ReturnsMutableVerticalTable);
  /// Use bitcompressed table
  param_member(bool, Compressed);
  /// Reference table used for type detection
  param_member(hyrise::storage::c_atable_ptr_t , ReferenceTable);
  /// save Properties/ColumnType in this struct...
  param_member(std::shared_ptr<ColumnProperties> , ColProperties);
public:
  params();
  ~params();
  params(const params &other);
  params &operator= (const params &other);
  params *clone() const;
};

std::shared_ptr<AbstractTable> load(const params &args);
std::shared_ptr<AbstractTable> generateValidityTable(std::shared_ptr<AbstractTable> table, hyrise::tx::transaction_id_t txid);
};

#endif  // SRC_LIB_IO_LOADER_H_
