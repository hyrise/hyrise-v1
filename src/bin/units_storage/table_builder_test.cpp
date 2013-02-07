// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"

#include <storage/TableBuilder.h>

class TableBuilderTest : public ::hyrise::Test {};

TEST_F(TableBuilderTest, simple_param) {
  TableBuilder::param p;
  p.set_type("STRING").set_name("test");

  ASSERT_EQ("STRING", p.type);
  ASSERT_EQ("test", p.name);
}

TEST_F(TableBuilderTest, build_param_list) {

  TableBuilder::param_list list;
  list.append().set_type("FLOAT").set_name("first");
  list.append().set_type("STRING").set_name("second");

  ASSERT_EQ(2u, list.params().size());

  TableBuilder::param p = list.params().front();
  ASSERT_EQ("FLOAT", p.type);
  ASSERT_EQ("first", p.name);

  p = list.params().back();
  ASSERT_EQ("STRING", p.type);
  ASSERT_EQ("second", p.name);
}

TEST_F(TableBuilderTest, cannot_allow_empty_table) {
  ASSERT_THROW(TableBuilder::build(TableBuilder::param_list()), TableBuilderError);
}

TEST_F(TableBuilderTest, build_table) {
  TableBuilder::param_list list;
  list.append().set_type("FLOAT").set_name("first");
  list.append().set_type("STRING").set_name("second");


  AbstractTable::SharedTablePtr  result = TableBuilder::build(list);
  ASSERT_TRUE((bool) result);
  ASSERT_EQ(2u, result->columnCount());
}


TEST_F(TableBuilderTest, build_table_with_layout) {

  TableBuilder::param_list list;
  list.append().set_type("FLOAT").set_name("first");
  list.append().set_type("STRING").set_name("second");
  list.append().set_type("INTEGER").set_name("third");


  // Set the layout
  list.appendGroup(1).appendGroup(2);

  AbstractTable::SharedTablePtr  result = TableBuilder::build(list);
  ASSERT_TRUE((bool) result);
  ASSERT_EQ(3u, result->columnCount());
  ASSERT_EQ(2u, result->sliceCount());
}

TEST_F(TableBuilderTest, build_table_with_layout_all_columns) {

  TableBuilder::param_list list;
  list.append().set_type("FLOAT").set_name("first");
  list.append().set_type("STRING").set_name("second");
  list.append().set_type("INTEGER").set_name("third");


  // Set the layout
  list.appendGroup(1).appendGroup(1).appendGroup(1);

  AbstractTable::SharedTablePtr  result = TableBuilder::build(list);
  ASSERT_TRUE((bool) result);
  ASSERT_EQ(3u, result->columnCount());
  ASSERT_EQ(3u, result->sliceCount());
}

TEST_F(TableBuilderTest, build_table_with_layout_all_row) {

  TableBuilder::param_list list;
  list.append().set_type("FLOAT").set_name("first");
  list.append().set_type("STRING").set_name("second");
  list.append().set_type("INTEGER").set_name("third");


  // Set the layout
  list.appendGroup(3);

  AbstractTable::SharedTablePtr  result = TableBuilder::build(list);
  ASSERT_TRUE((bool) result);
  ASSERT_EQ(3u, result->columnCount());
  ASSERT_EQ(1u, result->sliceCount());

}

TEST_F(TableBuilderTest, build_table_with_layout_order_check) {

  TableBuilder::param_list list;
  list.append().set_type("FLOAT").set_name("first");
  list.append().set_type("STRING").set_name("second");
  list.append().set_type("INTEGER").set_name("third");


  // Set the layout
  list.appendGroup(1).appendGroup(2);

  AbstractTable::SharedTablePtr  result = TableBuilder::build(list);
  ASSERT_TRUE((bool) result);
  ASSERT_EQ(3u, result->columnCount());
  ASSERT_EQ(2u, result->sliceCount());

}

TEST_F(TableBuilderTest, build_table_with_bad_layout_should_throw) {

  TableBuilder::param_list list;
  list.append().set_type("FLOAT").set_name("first");
  list.append().set_type("STRING").set_name("second");
  list.append().set_type("INTEGER").set_name("third");


  // Set the layout
  list.appendGroup(1).appendGroup(9);

  ASSERT_THROW(TableBuilder::build(list), TableBuilderError);
}
