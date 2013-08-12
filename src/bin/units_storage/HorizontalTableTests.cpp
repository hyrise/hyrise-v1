#include "testing/test.h"

#include "storage/Store.h"
#include "storage/MutableVerticalTable.h"
#include "storage/Table.h"
#include "storage/HorizontalTable.h"

#include "storage/TableBuilder.h"
#include "storage/storage_types.h"

namespace hyrise { namespace storage {

TEST(HorizontalTableTests, test_nesting) {
  TableBuilder::param_list list;
  list.append().set_type("INTEGER").set_name("first");
  auto part1 = TableBuilder::build(list);
  auto part2 = TableBuilder::build(list);
  part1->resize(1); 
  part1->setValue<hyrise_int_t>(0, 0, 1);
  part2->resize(2);
  part2->setValue<hyrise_int_t>(0, 0, 2);
  part2->setValue<hyrise_int_t>(0, 1, 3);

  std::vector<c_atable_ptr_t> tables {part1, part2};
  auto ht = std::make_shared<HorizontalTable>(tables);
  EXPECT_EQ(2u, ht->subtableCount());
  EXPECT_EQ(3u, ht->size());
  EXPECT_EQ(0u, ht->getValueId(0, 0).table);
  EXPECT_EQ(1u, ht->getValueId(0, 1).table);

  std::vector<c_atable_ptr_t> tables2 {ht, ht};
  /* Nested layout:
     part1 id: 0 
     part2 id: 1
     part1 id: 2
     part2 id: 3
  */
  auto nested_ht = std::make_shared<HorizontalTable>(tables2);
  EXPECT_EQ(4u, nested_ht->subtableCount());
  EXPECT_EQ(0u, nested_ht->getValueId(0, 0).table);
  EXPECT_EQ(1u, nested_ht->getValueId(0, 1).table);
  EXPECT_EQ(2u, nested_ht->getValueId(0, 3).table);
  EXPECT_EQ(3u, nested_ht->getValueId(0, 4).table);
}

}}
