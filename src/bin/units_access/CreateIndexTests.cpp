// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/CreateIndex.h"
#include "io/shortcuts.h"
#include "storage/InvertedIndex.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class CreateIndexTests : public AccessTest {};

TEST_F(CreateIndexTests, basic_create_index_test) {
  StorageManager *sm = StorageManager::getInstance();
  std::string table = "test_table1";
  std::shared_ptr<AbstractTable> t = Loader::shortcuts::load("test/index_test.tbl");

  CreateIndex i;
  i.addInput(t);
  i.addField(0);
  i.setTableName(table);
  i.execute();

  std::shared_ptr<InvertedIndex<hyrise_int_t>> index = std::dynamic_pointer_cast<InvertedIndex<hyrise_int_t>> (sm->getInvertedIndex(table));

  ASSERT_NE(index.get(), (InvertedIndex<storage::hyrise_int_t> *) NULL);
}

}
}
