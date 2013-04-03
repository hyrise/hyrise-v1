// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/InsertScan.h"

#include "storage/Store.h"

namespace hyrise {
namespace access {

InsertScan::~InsertScan() {
}

void InsertScan::executePlanOperation() {
  std::shared_ptr<const Store> store = std::dynamic_pointer_cast<const Store>(input.getTable(0));
  if (!store) {
    throw std::runtime_error("Insert without delta is not supported");
  }

  size_t max = store->getDeltaTable()->size();
  store->getDeltaTable()->resize(max + 1);
  store->getDeltaTable()->copyRowFrom(_data, 0, max, true);

  addResult(input.getTable(0));
}

const std::string InsertScan::vname() {
  return "InsertScan";
}

void InsertScan::setInputData(const storage::atable_ptr_t &c) {
  _data = c;
}

}
}
