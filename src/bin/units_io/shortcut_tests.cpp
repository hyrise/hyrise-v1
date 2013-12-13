// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <io/shortcuts.h>
#include <storage/AbstractTable.h>
#include <storage/Store.h>
#include <storage/RawTable.h>

namespace hyrise {
namespace io {

class LoaderShortcutTests : public ::hyrise::Test {};

TEST_F(LoaderShortcutTests, loadMainDelta) {
  auto s = Loader::shortcuts::loadMainDelta(
      "test/reference/update_scan_insert_only_after_update_main.tbl",
      "test/reference/update_scan_insert_only_after_update_delta.tbl"
                                            );
  ASSERT_EQ(3u, s->getMainTable()->size());
  ASSERT_EQ(1u, s->getDeltaTable()->size());
}

TEST_F(LoaderShortcutTests, loadShouldReturnStore) {
  hyrise::storage::atable_ptr_t  t = Loader::shortcuts::load("test/lin_xxs.tbl");
  ASSERT_TRUE((bool)std::dynamic_pointer_cast<storage::Store>(t));
}

TEST_F(LoaderShortcutTests, loadRawShouldReturnRawTable) {
  auto t = Loader::shortcuts::loadRaw("test/lin_xxs.tbl");
  ASSERT_TRUE((bool)std::dynamic_pointer_cast<storage::RawTable>(t));
  ASSERT_LT(0u, t->size());
}

} } // namespace hyrise::io

