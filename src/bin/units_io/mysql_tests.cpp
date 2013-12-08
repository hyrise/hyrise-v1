// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifdef WITH_MYSQL

#include "testing/test.h"
#include <stdlib.h>
#include <io/loaders.h>
#include "storage/AbstractTable.h"

#include <unistd.h>

namespace hyrise {
namespace io {

class MySQLTests : public ::hyrise::Test {
 protected:
  std::string _pid_suffix;
  std::string _schema;
  virtual void SetUp() override {
    _pid_suffix = std::to_string(getpid());
    _schema = "cbtr" + _pid_suffix;
    std::string cmd = "sh test/sap_data/load.sh " + _pid_suffix;
    system(cmd.c_str());
  }

  virtual void TearDown() override {
    std::string cmd = "sh test/sap_data/drop.sh " + _pid_suffix;
    system(cmd.c_str());
  }
};

TEST(MySQLTestsBase, load_test) {
  hyrise::storage::atable_ptr_t  t = Loader::load(
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

  std::vector<const char *> tables { "KNA1", "VBAP", "VBAK"};

  for(std::string table: tables) {
    hyrise::storage::atable_ptr_t  t = Loader::load(
        Loader::params().setInput(
            MySQLInput(
                MySQLInput::params()
                .setSchema(_schema)
                .setTable(table)
                       )
                                  )
                                                    );
    ASSERT_EQ(t->size(), 5u) << table << " should have 5 entries";
  }
}

TEST_F(MySQLTests, convert_date_to_int) {
  // Load SAP base schema, import KNA1, VBAP, VBAK into MySQL
  hyrise::storage::atable_ptr_t  t = Loader::load(
      Loader::params().setInput(
          MySQLInput(
              MySQLInput::params()
              .setSchema(_schema)
              .setTable("VBAK")
                     )));
  ASSERT_EQ(IntegerType, t->typeOfColumn(2));
  ASSERT_EQ(IntegerType, t->typeOfColumn(5));
}

} } // namespace hyrise::io

#endif
