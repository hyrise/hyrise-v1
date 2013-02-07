#include "access/Distinct.h"
#include "io/shortcuts.h"
#include "testing/test.h"
#include "storage/TableEqualityTest.h"

namespace hyrise {
namespace access {

class DistinctTests : public AccessTest {};

TEST_F(DistinctTests, basic_distinct_test) {
  std::shared_ptr<AbstractTable> t = Loader::shortcuts::load("test/tables/employees.tbl");
  std::shared_ptr<AbstractTable> reference = Loader::shortcuts::load("test/tables/employees_distinct.tbl");

  Distinct d;
  d.addInput(t);
  d.addField(1);
  d.execute();

  auto result = d.getResultTable();

  EXPECT_RELATION_EQ(result, reference);
}

TEST_F(DistinctTests, distinct_on_distinct_column_test) {
  std::shared_ptr<AbstractTable> t = Loader::shortcuts::load("test/tables/employees.tbl");

  Distinct d;
  d.addInput(t);
  d.addField(0);
  d.execute();

  auto result = d.getResultTable();

  EXPECT_RELATION_EQ(result, t);
}

}
}
