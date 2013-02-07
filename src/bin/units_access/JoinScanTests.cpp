#include "access/JoinScan.h"
#include "access/SimpleTableScan.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class JoinScanTests : public AccessTest {};

TEST_F(JoinScanTests, basic_join_scan_test) {
  std::shared_ptr<AbstractTable> t1 = Loader::shortcuts::load("test/join_transactions.tbl");
  std::shared_ptr<AbstractTable> t2 = Loader::shortcuts::load("test/join_exchange.tbl");
  std::shared_ptr<AbstractTable> reference = Loader::shortcuts::load("test/reference/join_result.tbl");

  JoinScan js(JoinType::EQUI);
  js.addInput(t1);
  js.addInput(t2);
  js.addCombiningClause(AND);
  js.addJoinClause<int>(0,0,1,0);
  js.addJoinClause<std::string>(0,1,1,1);
  js.execute();

  auto result = js.getResultTable();

  ASSERT_TRUE(result->contentEquals(reference));
}

}
}
