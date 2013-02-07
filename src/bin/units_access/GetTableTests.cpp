#include "access/GetTable.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class GetTableTests : public AccessTest {
public:
  GetTableTests() : AccessTest(), _table(Loader::shortcuts::load("test/empty.tbl")) {}
  std::shared_ptr<AbstractTable> _table;
};

TEST_F(GetTableTests, basic_get_table_test) {
  StorageManager::getInstance()->loadTable("new_table", _table);
  GetTable gt("new_table");
  ASSERT_EQ(_table, gt.execute()->getResultTable())
      << "Result table should be equal to the one loaded prior to operation";
}

TEST_F(GetTableTests, get_table_fail_test) {
  GetTable gt("non_existent");
  ASSERT_THROW({ gt.execute(); }, std::runtime_error);
}

}
}
