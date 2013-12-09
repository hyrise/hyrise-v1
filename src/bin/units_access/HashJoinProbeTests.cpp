// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/HashJoinProbe.h"
#include "access/HashBuild.h"

#include "io/shortcuts.h"
#include "storage/AbstractHashTable.h"
#include "testing/TableEqualityTest.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class HashJoinProbeTests : public AccessTest {};

TEST_F(HashJoinProbeTests, DISABLED_basic_hash_join_probe_test) {
  const std::string header_left("A|B|C\nINTEGER|STRING|FLOAT\n0_R|0_R|0_R");
  const std::string header_right("D|E|F\nINTEGER|STRING|FLOAT\n0_R|0_R|0_R");
  const std::string header_ref("A|B|C|D|E|F\nINTEGER|STRING|FLOAT|INTEGER|STRING|FLOAT\n0_R|0_R|0_R|0_R|0_R|0_R");
  auto left      = io::Loader::shortcuts::loadWithStringHeader("test/tables/hash_table_test.tbl", header_left);
  auto right     = io::Loader::shortcuts::loadWithStringHeader("test/tables/hash_table_test.tbl", header_right);
  auto reference = io::Loader::shortcuts::loadWithStringHeader("test/reference/hash_table_test_int.tbl", header_ref);

  HashBuild hb;
  hb.addInput(right);
  hb.addField(0);
  hb.setKey("join");
  hb.execute();

  const auto &right_hash = hb.getResultHashTable();

  HashJoinProbe hjp;
  hjp.addInput(left);
  hjp.addField(0);
  hjp.addInput(right_hash);
  hjp.execute();

  const auto &result = hjp.getResultTable();

  EXPECT_RELATION_EQ(result, reference);
}

}
}
