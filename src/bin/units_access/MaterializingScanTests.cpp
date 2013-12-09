// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MaterializingScan.h"
#include "access/ProjectionScan.h"
#include "io/shortcuts.h"
#include "storage/PointerCalculator.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class MaterializingScanTests : public AccessTest {};

TEST_F(MaterializingScanTests, basic_materializing_scan_test) {
  auto t = io::Loader::shortcuts::load("test/lin_xxs.tbl");
  auto no_p = (storage::PointerCalculator*) nullptr;

  ProjectionScan ps;
  ps.addInput(t);
  ps.addField(0);
  ps.setProducesPositions(true);
  ps.execute();

  const auto &t2 = ps.getResultTable();
  ASSERT_NE(no_p, dynamic_cast<const storage::PointerCalculator*>(t2.get()));

  MaterializingScan ms;
  ms.addInput(t2);
  ms.addField(0);
  ms.execute();

  const auto &result = ms.getResultTable();
  ASSERT_EQ(no_p, dynamic_cast<const storage::PointerCalculator*>(result.get()));
  ASSERT_EQ(100u, result->size());
  ASSERT_EQ(1u, result->columnCount());
}

}
}
