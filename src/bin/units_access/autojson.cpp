// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <iostream>
#include <algorithm>
#include <iterator>

#include <boost/filesystem.hpp>

#include "testing/test.h"
#include "helper.h"

#include "access/system/PlanOperation.h"
#include "io/TransactionManager.h"
#include "io/StorageManager.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

using namespace boost::filesystem;

#if GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::ValuesIn;

class AutoJsonTest : public TestWithParam<std::string> {
 public:
  AutoJsonTest() {
    sm = io::StorageManager::getInstance();
  }
  virtual ~AutoJsonTest() { }

  virtual void SetUp() {
    json_name = GetParam();
    sm->removeAll(); // Make sure old tables don't bleed into these tests
    tx::TransactionManager::getInstance().reset();
  }

  virtual void TearDown() {
    json_name = "";
  }

 protected:
  std::string json_name;
  io::StorageManager *sm;
};

TEST_P(AutoJsonTest, Query) {
  RecordProperty("JSONFile", json_name.c_str());

  std::string q = loadFromFile("test/autojson/" + json_name);

  auto has_xfail = json_name.find("xfail") != std::string::npos;

  hyrise::storage::c_atable_ptr_t out;
  try {
    out = executeAndWait(q);
  } catch (const std::runtime_error& e) {
    if (!has_xfail) {
      throw e; 
    }
  }
  
  if (sm->exists("reference")) {
    ASSERT_TRUE((bool)out);
    auto ref = io::StorageManager::getInstance()->getTable("reference");
    EXPECT_NE(ref.get(), out.get()) << "May not use 'reference' table to compare with itself";
    EXPECT_RELATION_EQ(ref, out);
  }
}

struct pathname_of {
  std::string operator()(const directory_entry &p) const {
    return p.path().filename().c_str();
  }
};

std::vector<std::string> GetParameterStrings() {
  std::vector<std::string> files;
  std::back_insert_iterator<std::vector<std::string> > back_it(files);
  std::transform(directory_iterator("test/autojson/"),
            directory_iterator(),
            back_it,
            pathname_of());

  std::vector<std::string> result;
  for (const auto& filename: files) {
    if ((filename.substr(filename.size() - 5).compare(".json") == 0)
        && (!(filename.substr(0, 3).compare("DIS")==0)))
      result.push_back(filename);
  }

  return result;
}

INSTANTIATE_TEST_CASE_P(
    AutoJson,
    AutoJsonTest,
    ValuesIn(GetParameterStrings())
                        );

#else
TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}
#endif

} } // namespace hyrise::access

