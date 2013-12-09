// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"
#include "testing/TableEqualityTest.h"

#include "access/storage/JsonTable.h"

#include <storage/Store.h>
#include "io/shortcuts.h"


namespace hyrise {
namespace access {

class JsonTableTest : public AccessTest {

protected:

  std::string _good_file;
  std::string _good_file_store;

  JsonTableTest(): _good_file("test/json/build_table_good.json"), _good_file_store("test/json/build_table_good_store.json") {
  }

};

TEST_F(JsonTableTest, simple_test) {

  JsonTable op;

  op.setNames({"A", "B", "C"});
  op.setTypes({"INTEGER", "STRING", "FLOAT"});
  op.setGroups({3});

  auto t = io::Loader::shortcuts::load("test/tables/radix_cluster_mpass.tbl");

  op.execute();
  auto result = op.getResultTable();

  EXPECT_RELATION_SCHEMA_EQ(t, result);
}

TEST_F(JsonTableTest, simple_test_with_data) {

  JsonTable op;

  op.setNames({"company_id", "company_name"});
  op.setTypes({"INTEGER", "STRING"});
  op.setGroups({1,1});
  op.setData({ {"1", "Apple Inc"}, {"2", "Microsoft"}, {"3", "SAP AG"}, {"4", "Oracle"}  });

  auto t = io::Loader::shortcuts::load("test/tables/companies.tbl");

  op.execute();
  auto result = op.getResultTable();

  EXPECT_RELATION_EQ(t, result);
}



TEST_F(JsonTableTest, builder_should_raise_in_case_of_errors_wrong_groups) {

  JsonTable op;

  op.setNames({"A", "B", "C"});
  op.setTypes({"INTEGER", "STRING", "FLOAT"});
  op.setGroups({3,3,3,3,3,3});

  auto t = io::Loader::shortcuts::load("test/tables/radix_cluster_mpass.tbl");
  ASSERT_ANY_THROW(op.execute());
}

TEST_F(JsonTableTest, builder_should_raise_in_case_of_errors_wrong_type) {

  JsonTable op;

  op.setNames({"A", "B", "C"});
  op.setTypes({"ISNTEGER", "STRING", "FLOAT"});
  op.setGroups({3,3,3});

  auto t = io::Loader::shortcuts::load("test/tables/radix_cluster_mpass.tbl");
  ASSERT_ANY_THROW(op.execute());
}

TEST_F(JsonTableTest, load_and_create_from_json) {
  auto data = loadFromFile(_good_file);
  auto result = executeAndWait(data);

  auto t = io::Loader::shortcuts::load("test/tables/radix_cluster_mpass.tbl");
  EXPECT_RELATION_SCHEMA_EQ(t, result);

  EXPECT_TRUE(std::dynamic_pointer_cast<const storage::Store>(result) == nullptr);
}

TEST_F(JsonTableTest, load_and_create_from_json_store) {
  auto data = loadFromFile(_good_file_store);
  auto result = executeAndWait(data);

  auto t = io::Loader::shortcuts::load("test/tables/radix_cluster_mpass.tbl");
  EXPECT_RELATION_SCHEMA_EQ(t, result);
  EXPECT_TRUE(std::dynamic_pointer_cast<const storage::Store>(result) != nullptr);
}

}
}
