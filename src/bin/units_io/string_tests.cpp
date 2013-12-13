// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include "io/loaders.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace io {

class StringLoaderTests : public ::hyrise::Test {};

TEST_F(StringLoaderTests, load_test) {
  hyrise::storage::atable_ptr_t  t = Loader::load(
      Loader::params()
      .setHeader(StringHeader("employee_id|employee_company_id|employee_name\n"
                              "INTEGER|INTEGER|STRING\n"
                              "0_C | 0_C | 0_C"))
      .setInput(CSVInput("test/tables/employees.data"))
                                                  );
}


TEST_F(StringLoaderTests, load_test_typesafe) {
  hyrise::storage::atable_ptr_t  t = Loader::load(
      Loader::params()
      .setHeader(StringHeader("employee_id|employee_company_id|employee_name\n"
                              "INTEGER|INTEGER|STRING\n"
                              "0_C | 0_C | 0_C"))
      .setInput(CSVInput("test/tables/employees.data"))
                                                  );


  EmptyInput input;
  StringHeader header("employee_id|employee_company_id|employee_name\n"
                      "INTEGER|INTEGER|INTEGER\n"
                      "0_C | 0_C | 0_C");

  Loader::params p;
  p.setInput(input).setHeader(header).setReturnsMutableVerticalTable(true).setReferenceTable(t);

  auto res = Loader::load(p);

  ASSERT_EQ(t->typeOfColumn(2), res->typeOfColumn(2));
}

} } // namespace hyrise::io

