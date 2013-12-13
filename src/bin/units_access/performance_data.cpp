// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include <io/shortcuts.h>
#include <access.h>

namespace hyrise {
namespace access {

class PerformanceDataTests : public AccessTest {};

TEST_F(PerformanceDataTests, single_op_data) {
  storage::atable_ptr_t w = io::Loader::shortcuts::loadWithHeader("test/regression/projection_fail.data", "test/regression/projection_fail.tbl");

  ProjectionScan ps;
  ps.addInput(w);
  ps.addField(w->numberOfColumn("w_tax"));

  performance_attributes_t perf;
  perf.startTime = 0;
  perf.endTime = 0;
  ps.setPerformanceData(&perf);

  ps.execute();

  ASSERT_GT(perf.startTime, 0u) << "start time should be set";
  ASSERT_GT(perf.endTime, 0u) << "end time should be set";
}

}
}

