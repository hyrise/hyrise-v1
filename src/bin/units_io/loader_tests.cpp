// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <io/CSVLoader.h>
#include <io/EmptyLoader.h>
#include <io/Loader.h>
#include <io/StringLoader.h>

#include <storage/MutableVerticalTable.h>
#include <storage/Store.h>

namespace hyrise {
namespace io {

class LoaderTests : public ::hyrise::Test {};

TEST_F(LoaderTests, load_with_empty_params_yields_empty_table) {
  hyrise::storage::atable_ptr_t  t = Loader::load(Loader::params());
  ASSERT_EQ(t->size(), 0u);
  ASSERT_EQ(t->columnCount(), 0u);
}

TEST_F(LoaderTests, wrong_inputs_should_fail) {
  CSVInput input("somefile");
  Loader::params p;
  p.setInput(input);
  ASSERT_THROW( {
      Loader::load(p);
    }, Loader::Error);
}

TEST_F(LoaderTests, load_csv_table_simple) {
  CSVInput input("test/test.csv", CSVInput::params().setCSVParams(csv::CSV_FORMAT));
  CSVHeader header("test/header.tbl", CSVHeader::params().setCSVParams(csv::CSV_FORMAT));
  hyrise::storage::atable_ptr_t  t = Loader::load(Loader::params().setInput(input)
                                                  .setHeader(header));
  ASSERT_TRUE((bool) std::dynamic_pointer_cast<storage::Store>(t));
  ASSERT_EQ(5u, t->columnCount());
  ASSERT_EQ(1u, t->size());
}

TEST_F(LoaderTests, load_vertical_table) {
  CSVInput input("test/test.csv", CSVInput::params().setCSVParams(csv::CSV_FORMAT));
  CSVHeader header("test/header.tbl", CSVHeader::params().setCSVParams(csv::CSV_FORMAT));
  hyrise::storage::atable_ptr_t  t = Loader::load(Loader::params().setInput(input).setHeader(header).setReturnsMutableVerticalTable(true));
  ASSERT_TRUE((bool)std::dynamic_pointer_cast<storage::MutableVerticalTable>(t)) << "t should be a vertical table";
}

TEST_F(LoaderTests, load_table_simple_empty) {
  EmptyInput input;
  CSVHeader header("test/header.tbl", CSVHeader::params().setCSVParams(csv::CSV_FORMAT));
  hyrise::storage::atable_ptr_t  t = Loader::load(Loader::params().setInput(input)
                                                  .setHeader(header));
  ASSERT_EQ(t->size(), 0u);
  ASSERT_EQ(t->columnCount(), 5u);
}

TEST_F(LoaderTests, load_string_header) {
  StringHeader header("a|b|c|d|e\nINTEGER|INTEGER|INTEGER|INTEGER|INTEGER\n0_R|0_R|1_R|2_R|3_R");
  hyrise::storage::atable_ptr_t  t =  Loader::load(Loader::params().setHeader(header));

  ASSERT_EQ(5u, t->columnCount());
  ASSERT_EQ(4u, t->partitionCount());
}

hyrise::storage::atable_ptr_t  loadTable() {
  CSVInput input("test/lin_xxs.tbl");
  CSVHeader header("test/lin_xxs.tbl");
  hyrise::storage::atable_ptr_t  t = Loader::load(
      Loader::params()
      .setInput(input)
      .setHeader(header)
                                                  );
  return t;
}

TEST_F(LoaderTests, load_table_hyrise_format) {
  hyrise::storage::atable_ptr_t  t = loadTable();
  ASSERT_EQ(t->size(), 100u);
}

} } // namespace hyrise::io

