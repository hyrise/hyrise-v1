// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <io/shortcuts.h>
#include <access.h>

#include <helper/epoch.h>

namespace hyrise {
namespace access {

class RegressionTests : public AccessTest {};

TEST_F(RegressionTests, projection_fail) {
  auto w = io::Loader::shortcuts::loadWithHeader("test/regression/projection_fail.data", "test/regression/projection_fail.tbl");

  ProjectionScan ps;
  ps.addInput(w);
  ps.addField(w->numberOfColumn("w_tax"));

  const auto& p = ps.execute()->getResultTable();
  ASSERT_EQ(1u, p->columnCount());
  ASSERT_EQ(p->metadataAt(0).getName(), "w_tax");
  ASSERT_EQ(0u, p->numberOfColumn("w_tax"));
  ASSERT_EQ(p->getValue<float>(0, 0), p->getValue<float>("w_tax", 0));

}

}
}
