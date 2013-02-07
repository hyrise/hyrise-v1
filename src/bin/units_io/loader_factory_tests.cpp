// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <io/loaders.h>
#include <storage/TableFactory.h>

class LoaderFactoryTests : public ::hyrise::Test {};

class MockTableFactory : public AbstractTableFactory {
public:
  int generate_call_cnt;
  MockTableFactory() : generate_call_cnt(0)  {}
  ~MockTableFactory() {}
  AbstractTable::SharedTablePtr  generate(std::vector<const ColumnMetadata *> *m,
                                          std::vector<AbstractTable::SharedDictionaryPtr> *d = nullptr,
                                          size_t initial_size = 0,
                                          bool sorted = true,
                                          bool compressed = false,
                                          size_t padding_size = STORAGE_ALIGNMENT_SIZE,
                                          size_t _align_size = STORAGE_ALIGNMENT_SIZE) {
    ++generate_call_cnt;
    return std::make_shared< Table<>>(m, d, initial_size, sorted, padding_size, _align_size);
  }
};

TEST_F(LoaderFactoryTests, load_test) {
  MockTableFactory mf;
  AbstractTable::SharedTablePtr  t = Loader::load(
                                       Loader::params().setHeader(CSVHeader("test/structured/1col_4rows.tbl"))
                                       .setFactory((AbstractTableFactory *) &mf)
                                     );
  ASSERT_EQ(mf.generate_call_cnt, 1) << "Generate should be called once";

}
