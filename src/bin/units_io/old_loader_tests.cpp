#include "testing/test.h"
#include "io/shortcuts.h"
#include "io/loaders.h"
#include "storage/AbstractTable.h"

class OldLoaderTests : public ::hyrise::Test {};

/*TEST_F(OldLoaderTests, generate_validity_table_from_tab) {
  AbstractTable::SharedTablePtr loadedTable = Loader::shortcuts::loadWithHeader("test/nonInsertOnly.data", "test/nonInsertOnly.tbl");
  AbstractTable::SharedTablePtr generatedTab = Loader::generateValidityTable(loadedTable);

  AbstractTable::SharedTablePtr reference = Loader::shortcuts::load("test/reference/InsertOnly.tbl");

  ASSERT_TRUE(generatedTab->contentEquals(reference));
  }*/

TEST_F(OldLoaderTests, safe_table_unsafe_load_set_false) {
  Loader::params p;
  p.setHeader(CSVHeader("test/header/revenue_header.tbl"));
  p.setInput(CSVInput("test/tables/revenue_data.tbl", CSVInput::params().setUnsafe(false)));
  AbstractTable::SharedTablePtr loadedTable = Loader::load(p);
  AbstractTable::SharedTablePtr reference = Loader::shortcuts::load("test/tables/revenue.tbl");
  ASSERT_TRUE(loadedTable->contentEquals(reference));

}

TEST_F(OldLoaderTests, safe_table_unsafe_load_set_true) {
  Loader::params p;
  p.setHeader(CSVHeader("test/header/revenue_header.tbl"));
  p.setInput(CSVInput("test/tables/revenue_data.tbl", CSVInput::params().setUnsafe(true)));
  AbstractTable::SharedTablePtr loadedTable = Loader::load(p);
  AbstractTable::SharedTablePtr reference = Loader::shortcuts::load("test/tables/revenue.tbl");
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
  AbstractTable::SharedTablePtr loadedTable = Loader::load(p);
  AbstractTable::SharedTablePtr reference = Loader::shortcuts::load("test/tables/revenue_small.tbl");
  ASSERT_TRUE(loadedTable->contentEquals(reference));
}
