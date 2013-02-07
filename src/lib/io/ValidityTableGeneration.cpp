// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "io/ValidityTableGeneration.h"

#include "storage/AbstractTable.h"
#include "storage/TableGenerator.h"

#include "helper/types.h"

namespace hyrise {
namespace insertonly {

AbstractTable::SharedTablePtr validityTableForTransaction(const size_t& rows,
                                                          tx::transaction_id_t from_tid,
                                                          tx::transaction_id_t to_tid) {
  TableGenerator generateTab(true);
  auto table = generateTab.create_empty_table(rows, {insertonly::VALID_FROM_COL_ID, insertonly::VALID_TO_COL_ID});
  for (size_t i = 0; i < rows;  ++i) {
    table->setValue<hyrise_int_t>(0, i, from_tid);
    table->setValue<hyrise_int_t>(1, i, to_tid);
  }
  return table;
}

}}
