// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifdef WITH_MYSQL

#include "testing/test.h"
#include <stdlib.h>
#include <io/loaders.h>
#include <boost/assign/list_inserter.hpp>
using boost::assign::push_back;

class MySQLTests : public ::hyrise::Test {};

TEST_F(MySQLTests, load_test) {
  AbstractTable::SharedTablePtr  t = Loader::load(
      Loader::params().setInput(
          MySQLInput(
              MySQLInput::params()
              .setSchema("information_schema")
              .setTable("TABLES")
                     )
                                )
                                                  );
}


TEST_F(MySQLTests, load_sap_schema) {
  // Load SAP base schema, import KNA1, VBAP, VBAK into MySQL
  system("sh -ex test/sap_data/load.sh");

  std::vector<const char *> tables;
  push_back(tables)("KNA1")("VBAP")("VBAK");

  for(std::string table: tables) {
    AbstractTable::SharedTablePtr  t = Loader::load(
        Loader::params().setInput(
            MySQLInput(
                MySQLInput::params()
                .setSchema("cbtr")
                .setTable(table)
                       )
                                  )
                                                    );
    ASSERT_EQ(t->size(), 5u) << table << " should have 5 entries";
  }
}

TEST_F(MySQLTests, convert_date_to_int) {
  // Load SAP base schema, import KNA1, VBAP, VBAK into MySQL
  system("sh -ex test/sap_data/load.sh");

  std::vector<const char *> tables;
  AbstractTable::SharedTablePtr  t = Loader::load(
      Loader::params().setInput(
          MySQLInput(
              MySQLInput::params()
              .setSchema("cbtr")
              .setTable("VBAK")
                     )));

  

  ASSERT_EQ(IntegerType, t->typeOfColumn(2));
  ASSERT_EQ(IntegerType, t->typeOfColumn(5));

  
}



#endif
