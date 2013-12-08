// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/HashBuild.h"
#include "io/shortcuts.h"
#include "storage/HashTable.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class HashBuildTests : public AccessTest {};

TEST_F(HashBuildTests, basic_hash_build_for_groupby_test) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(0);
  hb.setKey("groupby");
  hb.execute();

  const auto &result = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable>(hb.getResultHashTable());

  ASSERT_NE(result.get(), (storage::SingleAggregateHashTable *) nullptr);
}

TEST_F(HashBuildTests, basic_hash_build_for_join_test) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(0);
  hb.setKey("join");
  hb.execute();

  const auto &result = hb.getResultHashTable();

  ASSERT_NE(result.get(), (storage::JoinHashTable *) nullptr);
}

}
}
