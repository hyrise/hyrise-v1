#include "access/ExpressionScan.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class ExpressionScanTests : public AccessTest {};

TEST_F(ExpressionScanTests, basic_expression_scan_test) {
  std::shared_ptr<AbstractTable> t = Loader::shortcuts::load("test/lin_xxxs.tbl");
  std::shared_ptr<AbstractTable> reference = Loader::shortcuts::load("test/reference/simple_expression.tbl");

  AddExp plus(t, 0, 1);
  ExpressionScan es;
  es.addInput(t);
  es.setExpression("plus", &plus);
  es.execute();

  auto result = es.getResultTable();

  ASSERT_TRUE(result->contentEquals(reference));
}

}
}
