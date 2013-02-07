#include "testing/test.h"

#include <io/CSVLoader.h>
#include <io/Loader.h>

class LoaderExample : public ::hyrise::Test {};

TEST_F(LoaderExample, load_with_basepath) {
  AbstractTable::SharedTablePtr  t = Loader::load(
      Loader::params()
      .setInput(CSVInput("employees.data"))
      .setHeader(CSVHeader("employees.tbl"))
      .setBasePath("test/tables/")
                                                  );
}
