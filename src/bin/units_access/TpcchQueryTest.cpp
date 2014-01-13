// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/TableEqualityTest.h"
#include "io/StorageManager.h"
#include "helper.h"

namespace hyrise {
namespace access {

class TpcchQueryTest : public ::testing::TestWithParam<const char*> {
protected:
  virtual void SetUp() {
    io::ResourceManager::getInstance().clear();

    //convert parameters for test
    std::string numberString(GetParam());
    
    // load input tables
    executeAndWait(loadFromFile("test/tpcc/load_tpcc_tables.json"));

    // load expected output table
    hyrise::io::StorageManager *sm = hyrise::io::StorageManager::getInstance();
    sm->loadTableFile("refTable", "tpcch/query"+numberString+"_result.tbl");

    // load query from file
    query = loadFromFile("test/tpcch/query"+numberString+".json");
  }

  std::shared_ptr<hyrise::storage::AbstractTable> reference() {
    auto *sm = io::StorageManager::getInstance();
    return sm->getTable("refTable");
  }
  std::string query;
};

TEST_P(TpcchQueryTest, query_execute_test) {
  const auto& out = executeAndWait(this->query);

  ASSERT_TRUE(out != nullptr);

  EXPECT_RELATION_EQ(out, this->reference());
}

INSTANTIATE_TEST_CASE_P(TpcchQueryTestInstantiation, TpcchQueryTest, ::testing::Values("1","3","6","18","19"));

}
}