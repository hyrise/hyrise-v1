// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/radixjoin/Histogram.h"

#include "io/shortcuts.h"

#include "testing/test.h"
#include "testing/TableEqualityTest.h"

namespace hyrise {
namespace access {

class HistogramTest : public AccessTest {
public:
  HistogramTest() : AccessTest(), _table(io::Loader::shortcuts::load("test/tables/hash_table_test.tbl")) {}
  std::shared_ptr<storage::AbstractTable> _table;
};

TEST_F(HistogramTest, 2bit_test) {
	
  Histogram hst;
  hst.setBits(2);
  hst.addField(0);
  hst.addInput(_table);
  const auto& result = hst.execute()->getResultTable();

  EXPECT_EQ(3u, result->getValueId(0,0).valueId);
  EXPECT_EQ(4u, result->getValueId(0,1).valueId);
  EXPECT_EQ(1u, result->getValueId(0,2).valueId);
  EXPECT_EQ(0u, result->getValueId(0,3).valueId);
}

TEST_F(HistogramTest, 2bit_test_string) {
	
  Histogram hst;
  hst.setBits(2);
  hst.addField(1);
  hst.addInput(_table);
  const auto& result = hst.execute()->getResultTable();

  EXPECT_EQ(3u, result->getValueId(0,0).valueId);
  EXPECT_EQ(0u, result->getValueId(0,1).valueId);
  EXPECT_EQ(5u, result->getValueId(0,2).valueId);
  EXPECT_EQ(0u, result->getValueId(0,3).valueId);
}

TEST_F(HistogramTest, 1bit_test) {
	
  Histogram hst;
  hst.setBits(1);
  hst.addField(0);
  hst.addInput(_table);
  const auto& result = hst.execute()->getResultTable();

  EXPECT_EQ(4u, result->getValueId(0,0).valueId);
  EXPECT_EQ(4u, result->getValueId(0,1).valueId);
}

}}
