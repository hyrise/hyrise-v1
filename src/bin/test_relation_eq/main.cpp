#include <string>
#include <vector>
#include <iostream>

#include <io/Loader.h>
#include <io/CSVLoader.h>
#include <storage/AbstractTable.h>

#include "storage/TableEqualityTest.h"


int main (int argc, char** args) {
  AbstractTable::SharedTablePtr  t = Loader::load(
    Loader::params()
      .setHeader(CSVHeader("employees.tbl"))
      .setInput(CSVInput("employees.tbl"))
      .setBasePath("test/tables/"));

  std::vector<std::string> tables { "wrong1.tbl", "wrong2.tbl", "wrong3.tbl" };

  AbstractTable::SharedTablePtr tw;

  std::cout << std::endl;
  std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cout << "+++++the tests below are expected to fail++++++" << std::endl;
  std::cout << std::endl;

  for (unsigned int i = 0; i < tables.size(); i++) {
    tw = Loader::load( Loader::params()
        .setHeader(CSVHeader(tables[i]))
        .setInput(CSVInput(tables[i]))
        .setBasePath("test/tables/wrongs/"));
    std::cout << "****************TABLE::" << i+1 << "*****************" << std::endl;
    EXPECT_RELATION_EQ(t , tw);
    EXPECT_RELATION_EQ(tw, t);
  }

  std::cout << std::endl << std::endl;
  std::cout << "+++++nothing should went wrong from here on+++++" << std::endl;
  std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cout << std::endl;
}
