// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <unordered_map>

namespace hyrise {
namespace storage {
class AbstractTable;

typedef std::unordered_map<size_t, size_t> column_mapping_t;

column_mapping_t calculateMapping(const std::shared_ptr<const AbstractTable> &input,
                                  const std::shared_ptr<const AbstractTable> &dest);

column_mapping_t calculateMapping(const AbstractTable &input,
                                  const AbstractTable &dest);


}}

