// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <access/InsertScan.h>
#include <storage/storage_types.h>
#include <storage/Store.h>

InsertScan::~InsertScan() {
}

void InsertScan::setInputData(hyrise::storage::atable_ptr_t c) {
  data = c;
}

void InsertScan::executePlanOperation() {
  std::shared_ptr<const Store> store = std::dynamic_pointer_cast<const Store>(input.getTable(0));

  if (!store) {
    throw std::runtime_error("Insert without delta is not supported");
  }

  size_t max = store->getDeltaTable()->size();
  store->getDeltaTable()->resize(max + 1);
  store->getDeltaTable()->copyRowFrom(data, 0, max, true);

  // Since we hand the input table through we have to retain the
  // table, because otherwise our internal retain count would be
  // negative
  addResult(input.getTable(0));
}
