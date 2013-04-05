// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ProjectionScan.h"
#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class ProjectionScanTests : public AccessTest {};

TEST_F(ProjectionScanTests, basic_projection_scan_test) {
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");
  auto reference = Loader::shortcuts::load("test/reference/simple_projection.tbl");

  ProjectionScan ps;
  ps.addInput(t);
  ps.addField(0);
  ps.execute();

  const auto &result = ps.getResultTable();

  ASSERT_TRUE(result->contentEquals(reference));
}

TEST_F(ProjectionScanTests, projection_with_field_list_test) {
  auto t = Loader::shortcuts::load("test/lin_xxs.tbl");
  auto reference = Loader::shortcuts::load("test/reference/simple_projection.tbl");

  storage::field_list_t fields;
  fields.push_back(0);

  ProjectionScan ps(&fields);
  ps.addInput(t);
  ps.execute();

  const auto &result = ps.getResultTable();

  ASSERT_TRUE(result->contentEquals(reference));
}

}
}
