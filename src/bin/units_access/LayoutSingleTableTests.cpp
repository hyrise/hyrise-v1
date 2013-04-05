// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/Layouter.h"

#include <fstream>

#include "io/shortcuts.h"
#include "testing/test.h"

namespace hyrise {
namespace access {

class LayoutSingleTableTests : public AccessTest {
public:
  std::string loadFromFile(const std::string &path) {
    std::ifstream data_file(path.c_str());
    std::string result((std::istreambuf_iterator<char>(data_file)), std::istreambuf_iterator<char>());
    data_file.close();
    return result;
  }
};

TEST_F(LayoutSingleTableTests, basic_layout_single_table_test) {
  std::string header = loadFromFile("test/header/layouter_simple_cand.tbl");

  LayoutSingleTable::BaseQuery q;
  q.positions.push_back(0);
  q.selectivity = 1;
  q.weight = 1;

  LayoutSingleTable s;
  s.setLayouter(LayoutSingleTable::CandidateLayouter);
  s.addQuery(q);
  s.setNumRows(10000);
  s.addFieldName("A");
  s.addFieldName("B");
  s.addFieldName("C");
  s.execute();

  const auto &result = s.getResultTable();

  ASSERT_EQ(header, result->getValue<std::string>(0, 0));
  ASSERT_EQ(1u, result->size());
}

}
}
