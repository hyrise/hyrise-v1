// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/CreateIndex.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "storage/InvertedIndex.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class CreateIndexTests : public AccessTest {};

TEST_F(CreateIndexTests, basic_create_index_test) {
  auto sm = io::StorageManager::getInstance();
  std::string table = "test_table1";
  auto t = io::Loader::shortcuts::load("test/index_test.tbl");

  CreateIndex i;
  i.addInput(t);
  i.addField(0);
  i.setIndexName(table);
  i.execute();

  auto index = std::dynamic_pointer_cast<storage::InvertedIndex<hyrise_int_t>> (sm->getInvertedIndex(table));

  ASSERT_NE(index.get(), (storage::InvertedIndex<storage::hyrise_int_t> *) nullptr);
}

}
}
