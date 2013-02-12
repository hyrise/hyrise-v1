// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_TABLEUTILS_H_
#define SRC_LIB_STORAGE_TABLEUTILS_H_

#include <memory>
#include <unordered_map>

#include <helper/types.h>

class AbstractTable;

namespace hyrise {
namespace storage {

typedef std::unordered_map<size_t, size_t> column_mapping_t;

column_mapping_t calculateMapping(const hyrise::storage::c_atable_ptr_t &input,
                                  const hyrise::storage::c_atable_ptr_t &dest);

}}

#endif
