// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/AbstractIndex.h>

namespace hyrise {
namespace storage {

AbstractIndex::AbstractIndex() {};

AbstractIndex::AbstractIndex(std::string id) : _id(id) {}

AbstractIndex::~AbstractIndex() {}

std::string AbstractIndex::getId() { return _id; }

std::shared_ptr<AbstractIndex> AbstractIndex::recreateIndex(const c_atable_ptr_t& in, field_t column) {
  throw std::runtime_error("The index did not implement the recreateIndex method!");
}

std::shared_ptr<AbstractIndex> AbstractIndex::recreateIndexMergeDict(size_t column,
                                                                     std::shared_ptr<Store> store,
                                                                     std::shared_ptr<AbstractDictionary>& newDictReturn,
                                                                     std::vector<value_id_t>& x,
                                                                     std::vector<value_id_t>& Vd) {
  return nullptr;
}
}
}  // namespace hyrise::storage
