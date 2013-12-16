// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file AgingStore.h
 *
 * Contains the class definition of AgingStore.
 * For any undocumented method see AbstractTable.
 * @see AbstractTable
 */

#pragma once

#include <storage/Store.h>

namespace hyrise {
namespace storage {

/**
 * AgingStore consists of one or more main tables and a delta store and is the
 * only entity capable of modifying the content of the table(s) after
 * initialization via the delta store. It can be merged into the main
 * tables using a to-be-set merger.
 */
class AgingStore : public Store {
public:
  AgingStore();
  explicit AgingStore(atable_ptr_t main_table);
  virtual ~AgingStore();
};

} } // namespace hyrise::storage

