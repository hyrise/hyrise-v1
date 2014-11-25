// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <iterator>

#include <boost/filesystem.hpp>
#include <jsoncpp/json.h>

#include "testing/test.h"
#include "testing/TableEqualityTest.h"
#include "access/system/PlanOperation.h"
#include "io/TransactionManager.h"
#include "io/StorageManager.h"
#include "helper/HttpHelper.h"

#include "helper.h"



/**
 * Similar to autojson.cpp and adjusted for SQL
 */

namespace hyrise {
namespace access {

using namespace boost::filesystem;

#if GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::ValuesIn;



/**
 * AutoSQLTest class
 */
class AutoSQLTest : public TestWithParam<std::string> {
 public:
  AutoSQLTest() {}
  virtual ~AutoSQLTest() {}

  virtual void SetUp() {
    _sql_file_name = GetParam();
    io::StorageManager::getInstance()->removeAll();  // Make sure old tables don't bleed into these tests
    tx::TransactionManager::getInstance().reset();
  }

  virtual void TearDown() {
    _sql_file_name = "";
  }

 protected:
  std::string _sql_file_name;
};



/**
 * Test Case with parameters
 */
TEST_P(AutoSQLTest, Query) {
  std::string file = _sql_file_name.c_str();
  std::string file_contents = loadFromFile(file);


  // Parse test-json
  Json::Reader reader;
  Json::Value test_config;
  bool test_json_valid = reader.parse(file_contents, test_config);
  ASSERT_TRUE(test_json_valid);


  try {
    hyrise::storage::c_atable_ptr_t test_table, ref_table;
    
    // Execute prepare argument
    if (test_config.isMember("prepare")) {
      ASSERT_TRUE(test_config["prepare"].isString() || test_config["prepare"].isObject());

      if (test_config["prepare"].isString()) {
        executeSQLAndWait(test_config["prepare"].asString());
      } else if (test_config["prepare"].isObject()) {
        ref_table = executeJsonAndWait(test_config["reference"].toStyledString());
      }
    }

    // Execute test and reference arguments
    ASSERT_TRUE(test_config.isMember("test"));
    ASSERT_TRUE(test_config.isMember("reference"));
    ASSERT_TRUE(test_config["test"].isString());
    ASSERT_TRUE(test_config["reference"].isString() || test_config["reference"].isObject());

    std::string query = test_config["test"].asString();
    test_table = executeSQLAndWait(query);

    if (test_config["reference"].isString()) {
      ref_table = executeSQLAndWait(test_config["reference"].asString());
    } else if (test_config["reference"].isObject()) {
      ref_table = executeJsonAndWait(test_config["reference"].toStyledString());
    }

    // Evaluate results
    ASSERT_TRUE((bool) test_table);
    ASSERT_TRUE((bool) ref_table);
    EXPECT_RELATION_EQ(test_table, ref_table);
  } catch (std::runtime_error& e) {
    throw e;
  }
}


/**
 * Accessor for pathname
 */
struct pathname_of {
  std::string operator()(const directory_entry& p) const { return p.path().c_str(); }
};


/**
 * Creates list of .json files in the autosql directory
 */
std::vector<std::string> GetAutoSQLParameterStrings() {
  std::vector<std::string> files;
  std::back_insert_iterator<std::vector<std::string> > back_it(files);
  std::transform(directory_iterator("test/autosql/"), directory_iterator(), back_it, pathname_of());

  std::vector<std::string> result;
  for (const auto& filename : files) {
    if (filename.substr(filename.size() - 5).compare(".json") == 0)
      result.push_back(filename);
  }
  return result;
}


INSTANTIATE_TEST_CASE_P(AutoSQL, AutoSQLTest, ValuesIn(GetAutoSQLParameterStrings()));



#else
TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}
#endif

} // namespace access
} // namespace hyrise
