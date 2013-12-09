// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "io/shortcuts.h"
#include "io/loaders.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace io {

class OldLoaderTests : public ::hyrise::Test {};

/*TEST_F(OldLoaderTests, generate_validity_table_from_tab) {
  hyrise::storage::atable_ptr_t loadedTable = Loader::shortcuts::loadWithHeader("test/nonInsertOnly.data", "test/nonInsertOnly.tbl");
  hyrise::storage::atable_ptr_t generatedTab = Loader::generateValidityTable(loadedTable);

  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/InsertOnly.tbl");

  ASSERT_TRUE(generatedTab->contentEquals(reference));
  }*/

TEST_F(OldLoaderTests, safe_table_unsafe_load_set_false) {
  Loader::params p;
  p.setHeader(CSVHeader("test/header/revenue_header.tbl"));
  p.setInput(CSVInput("test/tables/revenue_data.tbl", CSVInput::params().setUnsafe(false)));
  hyrise::storage::atable_ptr_t loadedTable = Loader::load(p);
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/tables/revenue.tbl");
  ASSERT_TRUE(loadedTable->contentEquals(reference));

}

TEST_F(OldLoaderTests, safe_table_unsafe_load_set_true) {
  Loader::params p;
  p.setHeader(CSVHeader("test/header/revenue_header.tbl"));
  p.setInput(CSVInput("test/tables/revenue_data.tbl", CSVInput::params().setUnsafe(true)));
  hyrise::storage::atable_ptr_t loadedTable = Loader::load(p);
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/tables/revenue.tbl");
  ASSERT_TRUE(loadedTable->contentEquals(reference));

}

TEST_F(OldLoaderTests, unsafe_table_unsafe_load_set_false) {
  Loader::params p;
  p.setHeader(CSVHeader("test/header/revenue_modified_header.tbl"));
  p.setInput(CSVInput("test/tables/revenue_data.tbl", CSVInput::params().setUnsafe(false)));

  ASSERT_THROW( {
      Loader::load(p);
    }, Loader::Error);
}

TEST_F(OldLoaderTests, unsafe_table_unsafe_load_set_true) {
  Loader::params p;
  p.setHeader(CSVHeader("test/header/revenue_modified_header.tbl"));
  p.setInput(CSVInput("test/tables/revenue_data.tbl", CSVInput::params().setUnsafe(true)));
  hyrise::storage::atable_ptr_t loadedTable = Loader::load(p);
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/tables/revenue_small.tbl");
  ASSERT_TRUE(loadedTable->contentEquals(reference));
}

} } // namespace hyrise::io

