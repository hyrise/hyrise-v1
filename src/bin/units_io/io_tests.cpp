// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "io/loaders.h"
#include "io/shortcuts.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace io {

class IoTest : public ::hyrise::Test {

 protected:
  std::string header, header_fail, header_fail2, header_fail3;
  std::string fail_file_1;
  std::string fail_file_2, fail_file_3, fail_file_4;
  std::string file_not_found;

  virtual void SetUp() {
    header = "themen | epic\nINTEGER|INTEGER|INTEGER\n0_R|0_R";
    header_fail = "themen | epic\nINTEGER|INTEGER\n0_R|0_R|0_R";
    header_fail2 = "themen | epic\nINTEGER|INTEGER\n0_R|0_R_3";
    header_fail3 = "themen | epic\nINTEGER|INTEGER\n0_R|0_F";
    fail_file_1 = "test/fail.tbl";
    fail_file_2 = "test/fail2.tbl";
    fail_file_3 = "test/fail3.tbl";
    fail_file_4 = "test/fail4.tbl";
    file_not_found = "test/file_not_found.tbl";
  }

};


TEST_F(IoTest, OldLoader_fail_test) {
  ASSERT_THROW( {
      Loader::shortcuts::load(fail_file_1);
    }, Loader::Error);

}

TEST_F(IoTest, file_not_found) {
  ASSERT_THROW( {
      Loader::shortcuts::load(file_not_found);
    }, Loader::Error);
}

TEST_F(IoTest, wrong_header) {
  ASSERT_THROW( {
      Loader::shortcuts::loadWithStringHeader(fail_file_2, header);
    }, Loader::Error);
}

TEST_F(IoTest, wrong_header2) {
  ASSERT_THROW( {
      Loader::shortcuts::loadWithStringHeader(fail_file_2, header_fail);
    }, Loader::Error);
}

TEST_F(IoTest, wrong_header3) {
  ASSERT_THROW( {
      Loader::shortcuts::loadWithStringHeader(fail_file_2, header_fail);
    }, Loader::Error);
}

TEST_F(IoTest, wrong_file) {
  ASSERT_THROW( {
      Loader::shortcuts::loadWithStringHeader(fail_file_3, "themen | epic\nINTEGER|INTEGER\n0_R|0_R")->print();
    }, Loader::Error);

  // ASSERT_THROW({
  //         OldLoader::load_with_string_header(fail_file_4, "themen | epic\nINTEGER|INTEGER\n0_R|0_R");
  //     }, OldLoaderError);
}

} } // namespace hyrise::io

