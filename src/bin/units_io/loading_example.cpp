// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <io/CSVLoader.h>
#include <io/Loader.h>

namespace hyrise {
namespace io {

class LoaderExample : public ::hyrise::Test {};

TEST_F(LoaderExample, load_with_basepath) {
  hyrise::storage::atable_ptr_t  t = Loader::load(
      Loader::params()
      .setInput(CSVInput("employees.data"))
      .setHeader(CSVHeader("employees.tbl"))
      .setBasePath("test/tables/")
                                                  );
}

} } // namespace hyrise::io

