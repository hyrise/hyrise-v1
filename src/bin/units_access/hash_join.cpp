// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <algorithm>

#include "access/HashBuild.h"
#include "access/HashJoinProbe.h"
#include "helper/types.h"
#include "io/shortcuts.h"
#include "storage/AbstractTable.h"
#include "storage/AbstractHashTable.h"
#include "storage/Store.h"

#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

typedef std::shared_ptr<storage::AbstractTable> tbl_ptr;

storage::c_atable_ptr_t join(const tbl_ptr &left,
                             const tbl_ptr &right,
                             const std::vector<size_t> &fields_left,
                             const std::vector<size_t> &fields_right) {
  HashBuild hashBuild;
  hashBuild.addInput(left);
  hashBuild.setKey("join");
  for (auto & field_left: fields_left) hashBuild.addField(field_left);
  auto hashes = hashBuild.execute()->getResultHashTable();

  hyrise::access::HashJoinProbe hjp;  
  hjp.addInput(right);
  for (auto & field_right: fields_right) hjp.addField(field_right);
  hjp.addInput(hashes);

  return hjp.execute()->getResultTable();
}

const std::string header_a("A|B|C\nINTEGER|STRING|FLOAT\n0_R|0_R|0_R");
const std::string header_b("D|E|F\nINTEGER|STRING|FLOAT\n0_R|0_R|0_R");

typedef struct joinIdenticalParams {
  std::string name;
  std::vector<size_t> fields;
  std::string result;
} identicalJoinParams_t;

std::string PrintToString(const identicalJoinParams_t &t) {
  return t.name;
}


class HashTestJoinIdentical : public ::testing::TestWithParam<identicalJoinParams_t> {};

TEST_P(HashTestJoinIdentical, join_identical) {
  auto params = GetParam();
  auto left = io::Loader::shortcuts::loadWithStringHeader("test/tables/hash_table_test.tbl", header_a);
  auto right = io::Loader::shortcuts::loadWithStringHeader("test/tables/hash_table_test.tbl", header_b);

  auto result = join(left, right, params.fields, params.fields);
  auto reference = io::Loader::shortcuts::load(params.result);

  EXPECT_RELATION_EQ(result, reference);
}

class HashTestJoinIdenticalWithDelta : public ::testing::TestWithParam<identicalJoinParams_t> {};
/* TODO: Add string header_a and header_b
TEST_P(HashTestJoinIdenticalWithDelta, join_identical) {
  auto params = GetParam();
  auto store1 = Loader::shortcuts::loadMainDelta("test/tables/hash_table_test_main.tbl",
                "test/tables/hash_table_test_delta.tbl");
  auto store2 = Loader::shortcuts::loadMainDelta("test/tables/hash_table_test_main.tbl",
                "test/tables/hash_table_test_delta.tbl");

  //auto s = std::dynamic_pointer_cast<AbstractTable>(store);
  auto result = join(store1, store2, params.fields, params.fields);
  auto reference = Loader::shortcuts::load(params.result);

  EXPECT_RELATION_EQ(result, reference);
}
*/

const std::vector<identicalJoinParams_t> cases = {
  {"int", {0}, "test/reference/hash_table_test_int.tbl"},
  {"string", {1}, "test/reference/hash_table_test_string.tbl"},
  {"float", {2}, "test/reference/hash_table_test_float.tbl"},
  {"all", {0, 1, 2}, "test/reference/hash_table_test_all.tbl"}
};

INSTANTIATE_TEST_CASE_P(HashTestWithoutDelta,
                        HashTestJoinIdentical,
                        ::testing::ValuesIn(cases));


INSTANTIATE_TEST_CASE_P(HashTestWithDelta,
                        HashTestJoinIdenticalWithDelta,
                        ::testing::ValuesIn(cases));

} } // namespace hyrise::access

