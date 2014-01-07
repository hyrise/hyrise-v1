// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"

#include <iostream> //TODO

#define QUERY1 "test/aging/inv-q1.json"
#define QUERY2 "test/aging/inv-q2.json"
#define QUERY3 "test/aging/inv-q3.json"
#define QUERY4 "test/aging/inv-q4.json"
#define QUERY5 "test/aging/inv-q5.json"


namespace hyrise {
namespace access {

class AgingTests : public AccessTest {};

TEST_F(AgingTests , Q1_Customer3_2010) {
  parameter_map_t map;
  setParameter(map, "customer", "\"Customer3\"");
  setParameter(map, "year", 2010);

  const auto file = loadParameterized(QUERY1, map);
  executeAndWait(file);
}

} } // namespace hyrise::access

