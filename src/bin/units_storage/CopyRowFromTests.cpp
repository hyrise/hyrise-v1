// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "io/shortcuts.h"
#include "storage/AbstractTable.h"
#include "storage/Store.h"

#include <json.h>

namespace hyrise {
namespace storage {

class CopyRowFromTests : public ::hyrise::Test {};

TEST_F(CopyRowFromTests, json_test) {
  std::vector<Json::Value> data({123, 123, 123, 123, 123, 123, 123, 123, 123, 123});

  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto store = std::dynamic_pointer_cast<Store>(t);

  auto writeArea = store->appendToDelta(1);
  store->copyRowToDeltaFromJSONVector(data, writeArea.first, 0);

  for (size_t i = 0; i < data.size(); i++)
    ASSERT_EQ(123, store->getDeltaTable()->getValue<int>(i, 0));
}

TEST_F(CopyRowFromTests, string_test) {
  std::vector<std::string> data({"123", "123", "123", "123", "123", "123", "123", "123", "123", "123"});

  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto store = std::dynamic_pointer_cast<Store>(t);

  auto writeArea = store->appendToDelta(1);
  store->copyRowToDeltaFromStringVector(data, writeArea.first, 0);

  for (size_t i = 0; i < data.size(); i++)
    ASSERT_EQ(123, store->getDeltaTable()->getValue<int>(i, 0));
}
}
}
