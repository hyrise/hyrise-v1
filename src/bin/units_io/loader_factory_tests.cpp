// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <io/loaders.h>
#include <storage/TableFactory.h>

namespace hyrise {
namespace io {

class LoaderFactoryTests : public ::hyrise::Test {};

class MockTableFactory : public storage::AbstractTableFactory {
public:
  int generate_call_cnt;
  MockTableFactory() : generate_call_cnt(0)  {}
  ~MockTableFactory() {}
  storage::atable_ptr_t  generate(std::vector<storage::ColumnMetadata> *m,
                                  std::vector<storage::AbstractTable::SharedDictionaryPtr> *d = nullptr,
                                  size_t initial_size = 0,
                                  bool sorted = true,
                                  bool compressed = false) {
    ++generate_call_cnt;
    return std::make_shared<storage::Table>(m, d, initial_size, sorted, compressed);
  }
};

TEST_F(LoaderFactoryTests, load_test) {
  MockTableFactory mf;
  auto t = Loader::load(Loader::params().setHeader(CSVHeader("test/structured/1col_4rows.tbl"))
                        .setFactory((storage::AbstractTableFactory *) &mf));
  ASSERT_EQ(mf.generate_call_cnt, 1) << "Generate should be called once";

}

} } // namespace hyrise::io

