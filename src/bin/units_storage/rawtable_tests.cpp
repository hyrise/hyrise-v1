// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "testing/TableEqualityTest.h"

#include "io/shortcuts.h"
#include "storage/AbstractTable.h"
#include "storage/RawTable.h"
#include "storage/storage_types.h"
#include "storage/SimpleStore.h"
#include "storage/TableGenerator.h"
#include "storage/storage_types.h"


namespace hyrise {
namespace storage {
namespace rawtable {

class RawTableTests : public Test {
 public:
  metadata_vec_t intList(size_t num=2) {
    metadata_vec_t result;
    for(size_t i=0; i < num; ++i)
      result.push_back(ColumnMetadata::metadataFromString("INTEGER", "col" + std::to_string(i)));
    return result;
  }

  metadata_vec_t intstringlist() {
    metadata_vec_t result;
    result.push_back(ColumnMetadata::metadataFromString("INTEGER", "col1"));
    result.push_back(ColumnMetadata::metadataFromString("STRING", "col1"));
    return result;
  }

  
};

metadata_vec_t allTypeMeta() {
  return { ColumnMetadata::metadataFromString("INTEGER", "col_int"),
        ColumnMetadata::metadataFromString("STRING", "col_string"),
        ColumnMetadata::metadataFromString("FLOAT", "col_float")};
}


TEST_F(RawTableTests, test_raw_table_constructor) {
  auto cols = intList(2);
  RawTable main(cols);
}

TEST_F(RawTableTests, test_write) {
  auto cols = allTypeMeta();
  RawTable main(cols);
  auto toInsert = io::Loader::shortcuts::load("test/alltypes.tbl");
  main.appendRows(toInsert);
}

template <typename T>
class RawWriteTests : public ::hyrise::Test {};

template <int col, typename type>
struct TypeColumn {
  static const int column = col;
  typedef type columnType;
};

template <typename T>
struct testValues {
  static const std::vector<T> values;
};

template <>
struct testValues<hyrise_int_t> {
  static const std::vector<hyrise_int_t> values;
};

const std::vector<hyrise_int_t> testValues<hyrise_int_t>::values = { 0, 1, 2, 10, 20};

template <>
struct testValues<hyrise_float_t> {
  static const std::vector<hyrise_float_t> values;
};

const std::vector<hyrise_float_t> testValues<hyrise_float_t>::values = { .5f, 0.f, -.1f, .6f, 1.3f };

// the following types reflect the types of 'test/alltypes.tbl'

typedef TypeColumn<0, hyrise_int_t> int_column;
typedef TypeColumn<1, hyrise_string_t> string_column;
typedef TypeColumn<2, hyrise_float_t> float_column;

typedef ::testing::Types<int_column, float_column> writeable_columns;

TYPED_TEST_CASE(RawWriteTests, writeable_columns);

TYPED_TEST(RawWriteTests, write_default_value) {
  RawTable table(allTypeMeta());
  typedef typename TypeParam::columnType ColumnType;

  size_t column = TypeParam::column;
  const auto& values = testValues<ColumnType>::values;

  table.appendRows(io::Loader::shortcuts::load("test/alltypes.tbl"));
  for(size_t row=0; row < table.size(); ++row) {
    table.setValue(column, row, values.at(row));
  }

  for(size_t row=0; row < table.size(); ++row) {
    EXPECT_EQ(table.getValue<ColumnType>(column, row), values.at(row));
  }
}

TEST_F(RawTableTests, test_raw_table_record_builder) {
  auto cols = intList(2);
  hyrise::storage::rawtable::RowHelper rh(cols);

  rh.set<hyrise_int_t>(0, 99);
  rh.set<hyrise_int_t>(1, 838774);

  unsigned char * data = rh.build();
  ASSERT_EQ(24u, *((size_t*)data));
  ASSERT_EQ(99u, *((size_t*)data+1));
  ASSERT_EQ(838774u, *((size_t*)data+2));
  free(data);
  rh.reset();
}

TEST_F(RawTableTests, test_raw_table_record_builder_with_string) {
  auto cols = intstringlist();

  hyrise::storage::rawtable::RowHelper rh(cols);
  rh.set<hyrise_int_t>(0, 99);
  rh.set<hyrise_string_t>(1, "Martinistzukurz");

  unsigned char * data = rh.build();
  ASSERT_EQ(33u, *((size_t*)data));
  ASSERT_EQ(99u, *((size_t*)data+1));
  ASSERT_EQ(15u, *((unsigned short*)(data+16)));

  std::string tmp((char*)(data+18), 15);
  ASSERT_STREQ("Martinistzukurz", tmp.c_str());

  free(data);
  rh.reset();
}

TEST_F(RawTableTests, test_raw_table_record_builder_with_1m_rows) {
  auto cols = intstringlist();
  RawTable main(cols);

  for(size_t i=0; i < 1024*1024; ++i)
  {
      hyrise::storage::rawtable::RowHelper rh(cols);
      rh.set<hyrise_int_t>(0, i);
      rh.set<hyrise_string_t>(1, "Martinistzukurz");
      unsigned char *data = rh.build();
      main.appendRow(data);
      free(data);
  }
  ASSERT_EQ(1024u*1024u, main.size());
}

TEST_F(RawTableTests, test_raw_table_record_builder_with_1k_rows_and_get_op) {
  auto cols = intstringlist();
  RawTable main(cols);

  for(size_t i=0; i < 1024; ++i) {
    hyrise::storage::rawtable::RowHelper rh(cols);
    rh.set<hyrise_int_t>(0, i);
    rh.set<hyrise_string_t>(1, "Martinistzukurz");
    unsigned char *data = rh.build();
    main.appendRow(data);
    free(data);
  }

  for(int32_t i=1023; i >=0; --i) {
    hyrise::storage::rawtable::RowHelper rh(cols);
    rh.set<hyrise_int_t>(0, i);
    rh.set<hyrise_string_t>(1, "Martinistzukurz");
    unsigned char *data = rh.build();
    const unsigned char *md =  main.getRow(i);

    ASSERT_NE(data, md);
    ASSERT_EQ(0, memcmp(data, md, ((hyrise::storage::rawtable::record_header*) md)->width));
    free(data);
  }

  ASSERT_EQ(1024u, main.size());
}


TEST_F(RawTableTests, test_raw_table_record_builder_with_1k_rows_and_get_value) {
  auto cols = intstringlist();
  RawTable main(cols);

  for(size_t i=0; i < 1024; ++i) {
    hyrise::storage::rawtable::RowHelper rh(cols);
    rh.set<hyrise_int_t>(0, i);
    rh.set<hyrise_string_t>(1, "Martinistzukurz");
    unsigned char *data = rh.build();
    main.appendRow(data);
    free(data);
  }

  std::string cmpstr = "Martinistzukurz";
  for(int i=0; i < 1024; ++i) {
    ASSERT_EQ(i, main.getValue<hyrise_int_t>(0, i));
    ASSERT_STREQ(cmpstr.c_str(), main.getValue<hyrise_string_t>(1,i).c_str());
  }

 
  ASSERT_EQ(1024u, main.size());
}

TEST_F(RawTableTests, simple_store_initialize_and_should_behave_like_normal_table) {
  hyrise::storage::atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  hyrise::storage::atable_ptr_t tab = std::make_shared<hyrise::storage::SimpleStore>(t);
  ASSERT_TABLE_EQUAL(tab, t);
}

TEST_F(RawTableTests, simple_store_throws_unsopported) {
  hyrise::storage::atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  hyrise::storage::SimpleStore tab(t);

  ASSERT_THROW(tab.copy(), std::runtime_error);
}

TEST_F(RawTableTests, simple_store_insert_new_row_in_delta) {
  hyrise::storage::atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto tab = std::make_shared<hyrise::storage::SimpleStore>(t);
  auto delta = tab->getDelta();
  auto meta = delta->metadata();

  for(size_t i=0; i < 5; ++i) {
    hyrise::storage::rawtable::RowHelper rh(meta);
    for(size_t j=0; j < meta.size(); ++j) {
      rh.set<hyrise_int_t>(j, j*i);
    }
    unsigned char *data = rh.build();
    delta->appendRow(data);
    free(data);
  }

  ASSERT_EQ(delta->size(), 5u);
  ASSERT_THROW(tab->dictionaryByTableId(0, 1), std::runtime_error);
  ASSERT_THROW(tab->dictionaryAt(0, t->size()), std::runtime_error);
  ASSERT_THROW(tab->dictionaryAt(0, 0, 1), std::runtime_error);
}


TEST_F(RawTableTests, simple_store_insert_and_merge) {
  hyrise::storage::atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  hyrise::storage::atable_ptr_t ref = io::Loader::shortcuts::load("test/reference/lin_xxs_raw_merged.tbl");

  auto tab = std::make_shared<hyrise::storage::SimpleStore>(t);
  auto delta = tab->getDelta();
  auto meta = delta->metadata();

  for(size_t i=0; i < 5; ++i) {
    hyrise::storage::rawtable::RowHelper rh(meta);
    for(size_t j=0; j < meta.size(); ++j) {
      rh.set<hyrise_int_t>(j, j+i*meta.size());
    }
    unsigned char *data = rh.build();
    delta->appendRow(data);
    free(data);
  }

  tab->merge();
  EXPECT_RELATION_EQ(ref, tab);

}

TEST_F(RawTableTests, simple_store_insert_and_merge_new_values) {
  hyrise::storage::atable_ptr_t t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  hyrise::storage::atable_ptr_t ref = io::Loader::shortcuts::load("test/reference/lin_xxs_raw_merged2.tbl");

  auto tab = std::make_shared<hyrise::storage::SimpleStore>(t);
  auto delta = tab->getDelta();
  auto meta = delta->metadata();

  for(size_t i=0; i < 1; ++i) {
    hyrise::storage::rawtable::RowHelper rh(meta);
    for(size_t j=0; j < meta.size(); ++j) {
      rh.set<hyrise_int_t>(j, j+(i+100)*meta.size());
    }
    unsigned char *data = rh.build();
    delta->appendRow(data);
    free(data);
  }

  tab->merge();
  ASSERT_TABLE_EQUAL(ref, tab);
}

} } } // namespace hyrise::storage::rawtable

