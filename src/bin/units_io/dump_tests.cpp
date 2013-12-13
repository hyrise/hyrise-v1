// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"

#include <boost/filesystem.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include <io/CSVLoader.h>
#include <io/EmptyLoader.h>
#include <io/Loader.h>
#include <io/shortcuts.h>
#include <io/TableDump.h>
#include <storage/AbstractTable.h>
#include <storage/Store.h>
#include <storage/TableMerger.h>
#include <storage/AbstractMergeStrategy.h>
#include <storage/SequentialHeapMerger.h>
#include <storage/storage_types.h>

namespace hyrise {
namespace io {

class DumpTests : public ::hyrise::Test {

protected:

  hyrise::storage::atable_ptr_t simpleTable;

public:

  virtual void SetUp(){
    boost::filesystem::create_directories("./test/dump");
    simpleTable = Loader::shortcuts::load("test/lin_xxs.tbl");
  }

  virtual void TearDown() {
    boost::filesystem::remove_all("./test/dump");
  }

};

TEST_F(DumpTests, should_not_dump_other_tables_than_stores) {
  simpleTable = Loader::shortcuts::load("test/lin_xxs.tbl", Loader::params().setReturnsMutableVerticalTable(true));
  auto dumper = hyrise::storage::SimpleTableDump("./test/dump");
  ASSERT_THROW(dumper.dump("simple", simpleTable), std::runtime_error);
}

TEST_F(DumpTests, should_not_dump_store_with_muliple_generations) {

  auto *merger = new storage::TableMerger(new storage::DefaultMergeStrategy(), new storage::SequentialHeapMerger(), false);
  auto s = std::dynamic_pointer_cast<hyrise::storage::Store>(simpleTable);
  s->setMerger(merger);
  s->merge();

  s->resizeDelta(1);
  s->getDeltaTable()->setValue<hyrise_int_t>(0, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(4, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(5, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(6, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(7, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(8, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(9, 0, 1);
  s->merge();

  s->resizeDelta(1);
  s->getDeltaTable()->setValue<hyrise_int_t>(0, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(4, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(5, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(6, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(7, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(8, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(9, 0, 1);
  s->merge();

  auto dumper = hyrise::storage::SimpleTableDump("./test/dump");
  dumper.dump("simple", s);
}


TEST_F(DumpTests, simple_dump) {

  ASSERT_EQ(10u, simpleTable->columnCount());
  auto dumper = hyrise::storage::SimpleTableDump("./test/dump");
  auto result = dumper.dump("simple", simpleTable);
  ASSERT_TRUE(result);
}

TEST_F(DumpTests, simple_dump_load_header) {
  ASSERT_EQ(10u, simpleTable->columnCount());
  auto dumper = hyrise::storage::SimpleTableDump("./test/dump");
  auto result = dumper.dump("simple", simpleTable);
  ASSERT_TRUE(result);


  EmptyInput input;
  CSVHeader header("test/dump/simple/header.dat", CSVHeader::params().setCSVParams(csv::HYRISE_FORMAT));
  hyrise::storage::atable_ptr_t  t = Loader::load(Loader::params().setInput(input).setHeader(header));
  ASSERT_EQ(t->size(), 0u);
  ASSERT_EQ(t->columnCount(), simpleTable->columnCount());
}

TEST_F(DumpTests, simple_dump_load_all) {
  ASSERT_EQ(10u, simpleTable->columnCount());
  auto dumper = hyrise::storage::SimpleTableDump("./test/dump");
  auto result = dumper.dump("simple", simpleTable);
  ASSERT_TRUE(result);


  TableDumpLoader input("./test/dump", "simple");
  CSVHeader header("test/dump/simple/header.dat", CSVHeader::params().setCSVParams(csv::HYRISE_FORMAT));
  auto t = Loader::load(Loader::params().setInput(input).setHeader(header));
  ASSERT_EQ(t->size(), 100u);
  ASSERT_EQ(t->columnCount(), simpleTable->columnCount());
  ASSERT_TABLE_EQUAL(t, simpleTable);
}


TEST_F(DumpTests, simple_dump_should_not_dump_delta) {

  ASSERT_EQ(10u, simpleTable->columnCount());

  auto s = std::dynamic_pointer_cast<hyrise::storage::Store>(simpleTable);
  s->resizeDelta(1);
  s->getDeltaTable()->setValue<hyrise_int_t>(0, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(4, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(5, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(6, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(7, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(8, 0, 1);
  s->getDeltaTable()->setValue<hyrise_int_t>(9, 0, 1);

  ASSERT_EQ(101u, simpleTable->size());
  auto dumper = hyrise::storage::SimpleTableDump("./test/dump");
  auto result = dumper.dump("simple", simpleTable);
  ASSERT_TRUE(result);

  // Reload table
  simpleTable = Loader::shortcuts::load("test/lin_xxs.tbl");

  TableDumpLoader input("./test/dump", "simple");
  CSVHeader header("test/dump/simple/header.dat", CSVHeader::params().setCSVParams(csv::HYRISE_FORMAT));
  auto t = Loader::load(Loader::params().setInput(input).setHeader(header));
  ASSERT_EQ(t->size(), 100u);
  ASSERT_EQ(t->columnCount(), simpleTable->columnCount());
  ASSERT_TABLE_EQUAL(t, simpleTable);
}

} } // namespace hyrise::io

