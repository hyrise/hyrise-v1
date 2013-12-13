// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <io/loaders.h>

namespace hyrise {
namespace io {

class CSVTests : public ::hyrise::Test {};

TEST_F(CSVTests, load_test) {
  hyrise::storage::atable_ptr_t  t = Loader::load(
      Loader::params()
      .setHeader(CSVHeader("test/tables/employees.tbl"))
      .setInput(CSVInput("test/tables/employees.data"))
                                                  );
}


TEST_F(CSVTests, DISABLED_load_test_mpass) {
  hyrise::storage::atable_ptr_t  t = Loader::load(
      Loader::params()
      .setHeader(CSVHeader("test/loader/demo/demo.tbl"))
      .setInput(MPassCSVInput("test/loader/demo"))
                                                  );
}

} } // namespace hyrise::io

