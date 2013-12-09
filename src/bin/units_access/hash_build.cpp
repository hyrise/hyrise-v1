// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "access/HashBuild.h"
#include "storage/HashTable.h"
#include "access/MergeHashTables.h"
#include "storage/TableRangeView.h"
#include "helper.h"
#include "io/shortcuts.h"

namespace hyrise {
namespace access {

class HashBuildTest : public AccessTest {};

bool check_equality(const storage::c_ahashtable_ptr_t & ht1, const storage::c_ahashtable_ptr_t & ht2){
  bool isEqual = true;
  // check equality using the AbstractHashTable Interface
  // done by checking that size is equal and then iterate over both to check if pos_lists are equal
  isEqual = isEqual && (ht1->size(), ht2->size());
  isEqual = isEqual && (ht1->numKeys(), ht2->numKeys());
  //now get Table for one of the HashTables and
  auto table = ht1->getTable();
  auto fields = ht1->getFields();
  pos_list_t p1;
  pos_list_t p2;
  for(size_t i =0; i < table->size(); i++){
    p1 = ht1->get(table, fields, i);
    p2 = ht2->get(table, fields, i);
    std::sort(p1.begin(), p1.end());
    std::sort(p2.begin(), p2.end());
    isEqual = isEqual && (p1 == p2);
    if(!isEqual)
      break;
  }
  return isEqual;
}

TEST_F(HashBuildTest, check_equality) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(1);
  hb.setKey("groupby");
  hb.execute();
  auto hash = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(hb.getResultHashTable());

  auto t1 = storage::TableRangeView::create(t, 0, 4);
  auto t2 = storage::TableRangeView::create(t, 5, 9);

  HashBuild hb1;
  hb1.addInput(t1);
  hb1.addField(1);
  hb1.setKey("groupby");
  hb1.execute();
  auto hash1 = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(hb1.getResultHashTable());

  HashBuild hb2;
  hb2.addInput(t2);
  hb2.addField(1);
  hb2.setKey("groupby");
  hb2.execute();
  auto hash2 = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(hb2.getResultHashTable());

  ASSERT_TRUE(check_equality(hash, hash));
  ASSERT_FALSE(check_equality(hash1, hash2));
}

TEST_F(HashBuildTest, merge_one_table_test) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  HashBuild hb;
  hb.addInput(t);
  hb.addField(1);
  hb.setKey("groupby");
  hb.execute();
  auto hash1 = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(hb.getResultHashTable());

  MergeHashTables mht;
  mht.addInput(hash1);
  mht.setKey("groupby");
  mht.execute();
  auto hash2 = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(mht.getResultHashTable());


  ASSERT_TRUE(check_equality(hash1, hash2));
}

TEST_F(HashBuildTest, merge_two_tables_test) {
  // reference hash Table
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");
  HashBuild hb;
  hb.addInput(t);
  hb.addField(1);
  hb.setKey("groupby");
  hb.execute();
  auto hash = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(hb.getResultHashTable());

  //test to merge two tables
  auto t1 = storage::TableRangeView::create(t, 0, 5);
  auto t2 = storage::TableRangeView::create(t, 5, 10);

  HashBuild hb1;
  hb1.addInput(t1);
  hb1.addField(1);
  hb1.setKey("groupby");
  hb1.execute();
  auto hash1 = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(hb1.getResultHashTable());

  HashBuild hb2;
  hb2.addInput(t2);
  hb2.addField(1);
  hb2.setKey("groupby");
  hb2.execute();
  auto hash2 = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(hb2.getResultHashTable());

  MergeHashTables mht;
  mht.addInput(hash1);
  mht.addInput(hash2);
  mht.setKey("groupby");
  mht.execute();
  auto hash3 = std::dynamic_pointer_cast<const storage::SingleAggregateHashTable >(mht.getResultHashTable());

  ASSERT_EQ(hash->size(), hash3->size());
  ASSERT_EQ(hash->numKeys(), hash3->numKeys());
}
/*
TEST_F(HashBuildTest, performance_test) {
  // reference hash Table
  executeAndWait(loadFromFile("big_build.json"));
  ProfilerStart("/home/jwust/bigbuild.prof");
  executeAndWait(loadFromFile("big_build.json"));
  ProfilerStop();
  ProfilerStart("/home/jwust/bigbuild_p.prof");
  executeAndWait(loadFromFile("big_build_p.json"));
  ProfilerStop();
}*/
}
}
