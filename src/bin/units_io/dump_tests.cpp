// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"

#include <boost/filesystem.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include <access/CreateGroupkeyIndex.h>
#include <access/CreateDeltaIndex.h>
#include <access/IndexAwareTableScan.h>
#include <access/CompoundIndexScan.h>
#include <io/CSVLoader.h>
#include <io/EmptyLoader.h>
#include <io/Loader.h>
#include <io/shortcuts.h>
#include <io/StorageManager.h>
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
  virtual void SetUp() {
    boost::filesystem::create_directories("./test/dump");
    simpleTable = Loader::shortcuts::load("test/lin_xxs.tbl");
  }

  virtual void TearDown() { boost::filesystem::remove_all("./test/dump"); }
};

TEST_F(DumpTests, should_not_dump_other_tables_than_stores) {
  simpleTable = Loader::shortcuts::load("test/lin_xxs.tbl", Loader::params().setReturnsMutableVerticalTable(true));
  auto dumper = hyrise::storage::SimpleTableDump("./test/dump");
  ASSERT_THROW(dumper.dump("simple", simpleTable), std::runtime_error);
}

TEST_F(DumpTests, should_not_dump_store_with_muliple_generations) {

  auto* merger =
      new storage::TableMerger(new storage::DefaultMergeStrategy(), new storage::SequentialHeapMerger(), false);
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
  hyrise::storage::atable_ptr_t t = Loader::load(Loader::params().setInput(input).setHeader(header));
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

#ifdef PERSISTENCY_BUFFEREDLOGGER
TEST_F(DumpTests, dump_index_test) {
#else
TEST_F(DumpTests, DISABLED_dump_index_test) {
#endif
  auto* sm = StorageManager::getInstance();
  auto t1 = Loader::shortcuts::load("test/index_test.tbl");
  auto t2 = Loader::shortcuts::load("test/index_test.tbl");

  const std::string tableName = "DUMPTESTWITHINDEX";
  const std::string mainIndexName = "mcidx__" + tableName + "__main__" + t1->nameOfColumn(1);
  const std::string deltaIndexName = "mcidx__" + tableName + "__delta__" + t1->nameOfColumn(1);

  sm->loadTable(tableName, t1);
  sm->loadTable(tableName + "foo", t2);

  access::CreateGroupkeyIndex i1;
  i1.addInput(t1);
  i1.addField(1);
  i1.setIndexName(mainIndexName);
  i1.execute();
  access::CreateDeltaIndex di1;
  di1.addInput(t1);
  di1.addField(1);
  di1.setIndexName(deltaIndexName);
  di1.execute();

  access::CreateGroupkeyIndex i2;
  i2.addInput(t2);
  i2.addField(1);
  i2.setIndexName("mcidx__" + tableName + "foo__main__" + t2->nameOfColumn(1));
  i2.execute();
  access::CreateDeltaIndex di2;
  di2.addInput(t2);
  di2.addField(1);
  di2.setIndexName("mcidx__" + tableName + "foo__delta__" + t2->nameOfColumn(1));
  di2.execute();

  sm->persistTable(tableName);
  sm->removeTable(tableName);
  sm->removeTable(mainIndexName);
  sm->removeTable(deltaIndexName);
  sm->recoverTable(tableName);
  ASSERT_TRUE(sm->exists(tableName));
  ASSERT_TRUE(sm->exists(mainIndexName));
  ASSERT_TRUE(sm->exists(deltaIndexName));

  access::IndexAwareTableScan iats1;
  iats1.addInput(sm->getTable(tableName));
  iats1.setTableName(tableName);
  iats1.setPredicate(new access::GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(0, "col_1", 200));
  iats1.execute();

  access::IndexAwareTableScan iats2;
  iats2.addInput(sm->getTable(tableName));
  iats2.setTableName(tableName + "foo");
  iats2.setPredicate(new access::GenericExpressionValue<hyrise_int_t, std::equal_to<hyrise_int_t>>(0, "col_1", 200));
  iats2.execute();

  ASSERT_TABLE_EQUAL(iats2.getResultTable(), iats1.getResultTable());
}
}
}  // namespace hyrise::io
