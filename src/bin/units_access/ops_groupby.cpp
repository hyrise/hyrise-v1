// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <string>

#include "helper.h"

#include <access.h>
#include <helper/types.h>
#include <storage.h>

#include <io.h>
#include <io/shortcuts.h>
#include <helper/PapiTracer.h>

namespace hyrise {
namespace access {

storage::c_atable_ptr_t sort(storage::c_atable_ptr_t table, field_t field=0) {
  SortScan ss;
  ss.addInput(table);
  ss.setSortField(field);
  return ss.execute()->getResultTable();
}

class GroupByTests : public AccessTest {};

/*
TEST_F(GroupByTests, aggregate_group_by_scan_with_avg_on_integer) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/tables/revenue.tbl");

  GroupByScanPro gs;
  auto average = new AverageAggregateFun("amount");
  gs.addInput(t);
  gs.addField("year");
  gs.addFunction(average);

  HashBuildPro hs;
  hs.addInput(t);
  hs.addField("year");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  hyrise::storage::atable_ptr_t result = sort(gs.execute()->getResultTable());

  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/tables/revenue_average_per_year.tbl");
  ASSERT_TABLE_EQUAL(result, reference);
}


TEST_F(GroupByTests, aggregate_group_by_scan_with_count) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/10_30_group.tbl");

  auto count = new CountAggregateFun(0);

  GroupByScanPro gs;
  gs.addInput(t);
  gs.addFunction(count);
  gs.addField(0);

  HashBuildPro hs;
  hs.addInput(t);
  hs.addField(0);

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  hyrise::storage::atable_ptr_t result = gs.execute()->getResultTable();
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/group_by_scan_with_count.tbl");

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  hyrise::storage::atable_ptr_t r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  hyrise::storage::atable_ptr_t ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);

}


TEST_F(GroupByTests, aggregate_group_multi_table_with_delta_not_unique) {

  // Load raw data
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/groupby_xs_col.tbl");
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/group_by_delta_2.tbl");

  // make it modifiable
  auto s = std::make_shared< storage::Store>(t);

  // Do the insert
  s->getDeltaTable()->setValue<hyrise_int_t>(0, 0, 2008);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 0, 2);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 0, 50);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 0, 1);

  s->getDeltaTable()->setValue<hyrise_int_t>(0, 1, 2009);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 1, 2);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 1, 10);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 1, 1);

  auto sum = new SumAggregateFun(2);

  GroupByScanPro gs;
  gs.addInput(s);
  gs.addFunction(sum);
  gs.addField(0);

  HashBuildPro hs;
  hs.addInput(s);
  hs.addField(0);

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  hyrise::storage::atable_ptr_t result = gs.execute()->getResultTable();

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  hyrise::storage::atable_ptr_t r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  hyrise::storage::atable_ptr_t ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);

}


TEST_F(GroupByTests, aggregated_group_by_scan_using_table_2) {
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/10_30_group.tbl");

  {
    auto gs = std::make_shared<GroupByScanPro>();
    gs->addInput(t);

    gs->addField(0);

    auto hs = std::make_shared<HashBuildPro>();
    hs->addInput(t);
    hs->addField(0);

    auto group_map = hs->execute()->getResultHashTable();

    gs->addInput(group_map);

    hyrise::storage::atable_ptr_t result = gs->execute()->getResultTable();

    ASSERT_EQ(result->size(), t->size());


    GroupByScanPro gs2;
    gs2.addInput(t);

    gs2.addField(1);

    HashBuildPro hs2;
    hs2.addInput(t);
    hs2.addField(1);

    auto group_map2 = hs2.execute()->getResultHashTable();
    gs2.addInput(group_map2);

    result = gs2.execute()->getResultTable();

    SortScan s;
    s.addInput(result);


    s.setSortField(0);
    hyrise::storage::atable_ptr_t r2 = s.execute()->getResultTable();

    hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/group_by_scan_using_table_2.tbl");

    SortScan s2;
    s2.addInput(reference);
    s2.setSortField(0);
    hyrise::storage::atable_ptr_t ref2 = s2.execute()->getResultTable();

    ASSERT_TABLE_EQUAL(r2, ref2);
  }
  ASSERT_EQ(1u, t.use_count());
}




*/




TEST_F(GroupByTests, DISABLED_group_by_performance) {
  auto  t = io::Loader::shortcuts::load("test/test10k_12.tbl");

  GroupByScan gs;
  gs.addInput(t);
  gs.addField(0);

  HashBuild hs;
  hs.addInput(t);
  hs.addField(0);

  auto group_map = hs.execute()->getResultHashTable();

  gs.addInput(group_map);

  performance_attributes_t perf;
  gs.setPerformanceData(&perf);

  gs.execute();
  std::cout << perf.data << std::endl;
}

TEST_F(GroupByTests, group_by_scan_using_table_2) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  {
    auto gs = std::make_shared<GroupByScan>();
    gs->addInput(t);
    gs->addField(0);

    auto hs = std::make_shared<HashBuild>();
    hs->addInput(t);
    hs->addField(0);
    hs->setKey("groupby");

    auto group_map = hs->execute()->getResultHashTable();
    
    gs->addInput(group_map);

    const auto& result = gs->execute()->getResultTable();

    ASSERT_EQ(result->size(), t->size());


    hyrise::access::GroupByScan gs2;
    gs2.addInput(t);

    gs2.addField(1);

    hyrise::access::HashBuild hs2;
    hs2.addInput(t);
    hs2.addField(1);
    hs2.setKey("groupby");

    auto group_map2 = hs2.execute()->getResultHashTable();
    gs2.addInput(group_map2);

    const auto& result2 = gs2.execute()->getResultTable();

    SortScan s;
    s.addInput(result2);


    s.setSortField(0);
    const auto& r2 = s.execute()->getResultTable();

    auto reference = io::Loader::shortcuts::load("test/reference/group_by_scan_using_table_2.tbl");

    SortScan s2;
    s2.addInput(reference);
    s2.setSortField(0);
    const auto& ref2 = s2.execute()->getResultTable();

    ASSERT_TABLE_EQUAL(r2, ref2);
  }
  ASSERT_EQ(1u, t.use_count());
}

TEST_F(GroupByTests, group_by_scan_using_table_multiple_fields) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  hyrise::access::GroupByScan gs;
  gs.addInput(t);
  gs.addField(0);
  gs.addField(1);

  hyrise::access::HashBuild hs;
  hs.addInput(t);
  hs.addField(0);
  hs.addField(1);
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);
  const auto& result = gs.execute()->getResultTable();

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  const auto& r2 = so.execute()->getResultTable();


  const auto& reference = io::Loader::shortcuts::load("test/reference/group_by_scan_using_table_multiple_fields.tbl");

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  const auto& ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);
}

TEST_F(GroupByTests, group_by_scan_with_count) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  auto count = new CountAggregateFun(0);

  hyrise::access::GroupByScan gs;
  gs.addInput(t);
  gs.addFunction(count);
  gs.addField(0);

  hyrise::access::HashBuild hs;
  hs.addInput(t);
  hs.addField(0);
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  const auto& result = gs.execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/group_by_scan_with_count.tbl");

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  const auto& r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  const auto& ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);

}

TEST_F(GroupByTests, group_by_scan_with_sum) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  hyrise::access::GroupByScan gs;
  auto count = new SumAggregateFun(0);
  gs.addInput(t);
  gs.addFunction(count);
  const auto& result = gs.execute()->getResultTable();

  const auto& reference = io::Loader::shortcuts::load("test/reference/group_by_scan_with_sum.tbl");
  ASSERT_TABLE_EQUAL(result, reference);
}


TEST_F(GroupByTests, group_by_scan_with_avg_on_integer) {
  auto t = io::Loader::shortcuts::load("test/tables/revenue.tbl");

  hyrise::access::GroupByScan gs;
  auto average = new AverageAggregateFun("amount");
  gs.addInput(t);
  gs.addField("year");
  gs.addFunction(average);

  hyrise::access::HashBuild hs;
  hs.addInput(t);
  hs.addField("year");
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  const auto& result = sort(gs.execute()->getResultTable());

  const auto& reference = io::Loader::shortcuts::load("test/tables/revenue_average_per_year.tbl");
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(GroupByTests, group_by_scan_with_avg_on_float) {
  auto t = io::Loader::shortcuts::load("test/tables/revenue_float.tbl");

  hyrise::access::GroupByScan gs;
  auto average = new AverageAggregateFun("amount");
  gs.addInput(t);
  gs.addField("year");
  gs.addFunction(average);

  hyrise::access::HashBuild hs;
  hs.addInput(t);
  hs.addField("year");
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  const auto& result = sort(gs.execute()->getResultTable());

  const auto& reference = io::Loader::shortcuts::load("test/tables/revenue_average_per_year.tbl");
  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(GroupByTests, group_by_scan_with_avg_on_string) {
  auto t = io::Loader::shortcuts::load("test/tables/companies.tbl");

  hyrise::access::GroupByScan gs;
  auto average = new AverageAggregateFun(1);
  gs.addInput(t);
  gs.addField(0);
  gs.addFunction(average);

  hyrise::access::HashBuild hs;
  hs.addInput(t);
  hs.addField(0);
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  ASSERT_THROW( {
      gs.execute();
    }, std::runtime_error);
}

TEST_F(GroupByTests, group_by_scan_with_sum_and_two_args) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  hyrise::access::GroupByScan gs;
  auto count = new SumAggregateFun(0);
  gs.addInput(t);
  gs.addFunction(count);
  const auto& result = gs.execute()->getResultTable();

  const auto& reference = io::Loader::shortcuts::load("test/reference/group_by_scan_with_sum.tbl");

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  const auto& r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  const auto& ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);

}

TEST_F(GroupByTests, group_by_scan_with_count_and_two_args) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  hyrise::access::GroupByScan gs;
  auto count = new CountAggregateFun(0);
  gs.addInput(t);
  gs.addFunction(count);
  gs.addField(0);
  gs.addField(1);

  hyrise::access::HashBuild hs;
  hs.addInput(t);
  hs.addField(0);
  hs.addField(1);
  hs.setKey("groupby");

  
  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  const auto& result = gs.execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/group_by_scan_with_count_and_two_args.tbl");

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  const auto& r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  const auto& ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);
}


TEST_F(GroupByTests, group_by_scan_with_sum_and_without_grouping_field) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  auto sum = new SumAggregateFun(0);

  hyrise::access::GroupByScan gs;
  gs.addInput(t);
  gs.addFunction(sum);

  const auto& result = gs.execute()->getResultTable();
  const auto& reference = io::Loader::shortcuts::load("test/reference/group_by_scan_without_grouping_field.tbl");

  ASSERT_TABLE_EQUAL(result, reference);
}

TEST_F(GroupByTests, group_by_scan_with_sum_and_one_arg) {
  auto t = io::Loader::shortcuts::load("test/10_30_group.tbl");

  auto sum = new SumAggregateFun(0);

  hyrise::access::GroupByScan gs;
  gs.addInput(t);
  gs.addFunction(sum);
  gs.addField(1);

  hyrise::access::HashBuild hs;
  hs.addInput(t);
  hs.addField(1);
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  const auto& result = gs.execute()->getResultTable();

  const auto& reference = io::Loader::shortcuts::load("test/reference/group_by_scan_with_sum_and_one_arg.tbl");

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  const auto& r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  const auto& ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);

}

TEST_F(GroupByTests,  group_multi_table) {

  // Load raw data
  auto s = io::Loader::shortcuts::load("test/10_30_group.tbl");
  auto reference = io::Loader::shortcuts::load("test/reference/group_by_scan_with_sum_and_one_arg.tbl");

  auto sum = new SumAggregateFun(0);

  hyrise::access::GroupByScan gs;
  gs.addInput(s);
  gs.addFunction(sum);
  gs.addField(1);

  hyrise::access::HashBuild hs;
  hs.addInput(s);
  hs.addField(1);
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  const auto& result = gs.execute()->getResultTable();

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  const auto& r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  const auto& ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);

}

/*
TEST_F(GroupByTests, group_multi_table_with_delta) {

  // Load raw data
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/groupby_xs_col.tbl");
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/group_by_delta.tbl");

  // make it modifiable
  auto s = std::make_shared<storage::Store>(t);

  // Do the insert
  s->getDeltaTable()->setValue<hyrise_int_t>(0, 0, 2013);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 0, 2);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 0, 50);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 0, 1);

  s->getDeltaTable()->setValue<hyrise_int_t>(0, 1, 2014);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 1, 2);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 1, 10);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 1, 1);

  auto sum = new SumAggregateFun(2);

  s->print();

  hyrise::access::GroupByScan gs;
  gs.addInput(s);
  gs.addFunction(sum);
  gs.addField(0);

  hyrise::access::HashBuild hs;
  hs.addInput(s);
  hs.addField(0);
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  hyrise::storage::atable_ptr_t result = gs.execute()->getResultTable();

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  hyrise::storage::atable_ptr_t r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  hyrise::storage::atable_ptr_t ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);


}

TEST_F(GroupByTests, group_multi_table_with_delta_not_unique) {

  // Load raw data
  hyrise::storage::atable_ptr_t t = Loader::shortcuts::load("test/groupby_xs_col.tbl");
  hyrise::storage::atable_ptr_t reference = Loader::shortcuts::load("test/reference/group_by_delta_2.tbl");

  // make it modifiable
  auto s = std::make_shared< storage::Store>(t);

  // Do the insert
  s->getDeltaTable()->setValue<hyrise_int_t>(0, 0, 2008);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 0, 2);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 0, 50);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 0, 1);

  s->getDeltaTable()->setValue<hyrise_int_t>(0, 1, 2009);
  s->getDeltaTable()->setValue<hyrise_int_t>(1, 1, 2);
  s->getDeltaTable()->setValue<hyrise_int_t>(2, 1, 10);
  s->getDeltaTable()->setValue<hyrise_int_t>(3, 1, 1);

  auto sum = new SumAggregateFun(2);

  hyrise::access::GroupByScan gs;
  gs.addInput(s);
  gs.addFunction(sum);
  gs.addField(0);

  hyrise::access::HashBuild hs;
  hs.addInput(s);
  hs.addField(0);
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  hyrise::storage::atable_ptr_t result = gs.execute()->getResultTable();

  SortScan so;
  so.addInput(result);
  so.setSortField(0);
  hyrise::storage::atable_ptr_t r2 = so.execute()->getResultTable();

  SortScan so2;
  so2.addInput(reference);
  so2.setSortField(0);
  hyrise::storage::atable_ptr_t ref2 = so2.execute()->getResultTable();

  ASSERT_TABLE_EQUAL(r2, ref2);

}
*/

TEST_F(GroupByTests, groupby_on_empty_table) {
  storage::metadata_list metaList = { storage::ColumnMetadata("field1", IntegerType),
                                      storage::ColumnMetadata("field2", StringType),
                                      storage::ColumnMetadata("field2", FloatType) };
  auto t = std::make_shared<storage::Table>(&metaList);

  GroupByScan gs;
  gs.addInput(t);
  gs.addField(0);
  
  gs.addFunction(new SumAggregateFun("field1"));
  gs.addFunction(new AverageAggregateFun("field1"));
  gs.addFunction(new CountAggregateFun("field1", false));
  gs.addFunction(new CountAggregateFun("field1", true));
  gs.addFunction(new MinAggregateFun("field1"));
  gs.addFunction(new MaxAggregateFun("field1"));

  HashBuild hs;
  hs.addInput(t);
  hs.addField(0);
  hs.setKey("groupby");

  auto group_map = hs.execute()->getResultHashTable();
  gs.addInput(group_map);

  hyrise::storage::c_atable_ptr_t result;
  EXPECT_NO_THROW(result = gs.execute()->getResultTable());
  EXPECT_EQ(0u, result->size());
}

}
}

