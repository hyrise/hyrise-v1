#include "access/HashBuild.h"
#include "io/shortcuts.h"
#include "storage/HashTable.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class HashBuildTests : public AccessTest {};

TEST_F(HashBuildTests, basic_hash_build_for_groupby_test) {
  auto t = Loader::shortcuts::load("test/10_30_group.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(0);
  hb.setKey("groupby");
  hb.execute();

  auto result = std::dynamic_pointer_cast<AggregateHashTable>(hb.getResultHashTable());

  ASSERT_NE(result.get(), (AggregateHashTable *) NULL);
}

TEST_F(HashBuildTests, basic_hash_build_for_join_test) {
  auto t = Loader::shortcuts::load("test/10_30_group.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(0);
  hb.setKey("join");
  hb.execute();

  auto result = hb.getResultHashTable();

  ASSERT_NE(result.get(), (JoinHashTable *) NULL);
}

}
}
