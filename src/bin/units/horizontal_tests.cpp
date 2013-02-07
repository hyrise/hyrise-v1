#include "testing/base_test.h"
#include <string>
#include <vector>

///class HorizontalTest : public ::hyrise::Test {};

///*
//  This is the most simple test case for horizontal partitioning
// */
//TEST_F( HorizonalTest,simple_data_test )
//{
//
//      SimplePartitionSpec s = {0,0, 1,10};
//
//      PartitionSpec spec;
//      spec.push_back(s);
//
//
//      // Prepare
//      std::vector<ColumnMetadata*> m;
//      m.push_back(new ColumnMetadata<int>("int1", IntegerType));
//
//      ChunkedTable t(spec, m);
//
//      ASSERT_EQ(1, t.columnCount());
//
//}
//
//TEST_F( HorizonalTest,simple_vertical_data_test )
//{
//
//      SimplePartitionSpec s = {0,0,1,10};
//      SimplePartitionSpec s2 = {1,0,1,10};
//
//      PartitionSpec spec;
//      spec.push_back(s);
//      spec.push_back(s2);
//
//      // Prepare
//      std::vector<ColumnMetadata*> m;
//      m.push_back(new ColumnMetadata<int>("int1", IntegerType));
//      m.push_back(new ColumnMetadata<int>("int2", IntegerType));
//
//      ChunkedTable t(spec, m);
//
//      ASSERT_EQ(2, t.columnCount());
//
//}
//
//TEST_F( HorizonalTest,simple_horizontal_data_test )
//{
//
//      SimplePartitionSpec s = {0,0,1,10};
//      SimplePartitionSpec s2 = {0,10,1,10};
//
//      PartitionSpec spec;
//      spec.push_back(s);
//      spec.push_back(s2);
//
//      // Prepare
//      std::vector<ColumnMetadata*> m;
//      m.push_back(new ColumnMetadata<int>("int1", IntegerType));
//
//      ChunkedTable t(spec, m);
//
//      ASSERT_EQ(1, t.columnCount());
//}
//
//TEST_F( HorizonalTest,simple_vertical_and_horizontal_data_test )
//{
//
//      SimplePartitionSpec s = {0,0,1,10};
//      SimplePartitionSpec s2 = {0,10,1,10};
//      SimplePartitionSpec s3 = {0,1,1,20};
//      SimplePartitionSpec s4 = {0,2,1,20};
//
//      PartitionSpec spec;
//      spec.push_back(s);
//      spec.push_back(s2);
//      spec.push_back(s3);
//      spec.push_back(s4);
//
//      // Prepare
//      std::vector<ColumnMetadata*> m;
//      m.push_back(new ColumnMetadata<int>("int1", IntegerType));
//      m.push_back(new ColumnMetadata<int>("int2", IntegerType));
//      m.push_back(new ColumnMetadata<int>("int3", IntegerType));
//
//      ChunkedTable t(spec, m);
//
//      ASSERT_EQ(3, t.columnCount());
//
//      ASSERT_EQ("int1", t.nameOfColumn(0));
//      ASSERT_EQ("int2", t.nameOfColumn(1));
//      ASSERT_EQ("int3", t.nameOfColumn(2));
//}
//
//TEST_F( HorizonalTest,simple_vertical_and_horizontal_test_with_values )
//{
//
//      SimplePartitionSpec s = {0,0,1,10};
//      SimplePartitionSpec s2 = {0,10,1,10};
//      SimplePartitionSpec s3 = {1,0,1,20};
//      SimplePartitionSpec s4 = {2,0,1,20};
//
//      PartitionSpec spec;
//      spec.push_back(s);
//      spec.push_back(s2);
//      spec.push_back(s3);
//      spec.push_back(s4);
//
//      // Prepare
//      std::vector<ColumnMetadata*> m;
//      m.push_back(new ColumnMetadata<int>("int1", IntegerType));
//      m.push_back(new ColumnMetadata<int>("int2", IntegerType));
//      m.push_back(new ColumnMetadata<int>("int3", IntegerType));
//
//      ChunkedTable t(spec, m);
//
//      // Set some values
//      t.setValue<int>(0,0,1);
//      t.setValue<int>(1,0,2);
//      t.setValue<int>(2,0,3);
//      t.setValue<int>(0,10,10);
//      t.setValue<int>(1,10,22);
//      t.setValue<int>(2,10,33);
//
//
//      ASSERT_EQ(1, t.getValue<int>(0,0));
//      ASSERT_EQ(2, t.getValue<int>(1,0));
//      ASSERT_EQ(3, t.getValue<int>(2,0));
//      ASSERT_EQ(10, t.getValue<int>(0,10));
//      ASSERT_EQ(22, t.getValue<int>(1,10));
//      ASSERT_EQ(33, t.getValue<int>(2,10));
//
//      // Check size
//      size_t sz = t.size();
//      ASSERT_EQ(20, sz);
//
//}

