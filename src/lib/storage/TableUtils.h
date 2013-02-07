// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_TABLEUTILS_H_
#define SRC_LIB_STORAGE_TABLEUTILS_H_

#include <memory>
#include <unordered_map>

class AbstractTable;

namespace hyrise {
namespace storage {

typedef std::unordered_map<size_t, size_t> column_mapping_t;

column_mapping_t calculateMapping(const std::shared_ptr<const AbstractTable> &input,
                                  const std::shared_ptr<const AbstractTable> &dest);

column_mapping_t calculateMapping(const AbstractTable &input,
                                  const AbstractTable &dest);


}}

#endif
