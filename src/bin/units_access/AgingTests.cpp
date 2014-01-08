// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"


#define QUERY1 "test/aging/inv-q1.json"
#define QUERY2 "test/aging/inv-q2.json"
#define QUERY3 "test/aging/inv-q3.json"
#define QUERY4 "test/aging/inv-q4.json"
#define QUERY5 "test/aging/inv-q5.json"

namespace hyrise {
namespace access {

class AgingTests : public Test {
 public:
  virtual void SetUp() {}

  virtual void TearDown() {
  }
};


#define executeQuery1(customer, year, shouldBeHot) \
{\
  parameter_map_t map;\
  setParameter(map, "customer", customer);\
  setParameter(map, "year", year);\
\
  const auto file = loadParameterized(QUERY1, map);\
  executeAndWait(file);\
}

TEST_F(AgingTests , Query1) {
//executeQuery1(customer        , year, shouldBeHot);
  executeQuery1("\"Customer3\"" , 2010, true       );
  executeQuery1("\"Customer10\"", 2013, true       );
  executeQuery1("\"Customer12\"", 2013, false      );
}


#define executeQuery2(customer, status, shouldBeHot) \
{\
  parameter_map_t map;\
  setParameter(map, "customer", customer);\
  setParameter(map, "status", status);\
\
  const auto file = loadParameterized(QUERY2, map);\
  executeAndWait(file);\
}

TEST_F(AgingTests , Query2) {
//executeQuery2(customer        , status          , shouldBeHot);
  executeQuery2("\"Customer1\"" , "\"open\""      , true       );
  executeQuery2("\"Customer11\"", "\"in process\"", true       );
  executeQuery2("\"Customer12\"", "\"in process\"", false      );
}


#define executeQuery3(status, year, shouldBeHot) \
{\
  parameter_map_t map;\
  setParameter(map, "status", status);\
  setParameter(map, "year", year);\
\
  const auto file = loadParameterized(QUERY3, map);\
  executeAndWait(file);\
}

TEST_F(AgingTests , Query3) {
//executeQuery3(customer        , year, shouldBeHot);
  executeQuery3("\"on hold\""   , 2013, true       );
  executeQuery3("\"on hold\""   , 2013, true       );
  executeQuery3("\"in process\"", 2014, false      );
}


#define executeQuery4(customer, year, status, shouldBeHot) \
{\
  parameter_map_t map;\
  setParameter(map, "customer", customer);\
  setParameter(map, "year", year);\
  setParameter(map, "status", customer);\
\
  const auto file = loadParameterized(QUERY4, map);\
  executeAndWait(file);\
}

TEST_F(AgingTests , Query4) {
//executeQuery4(customer        , year, status          , shouldBeHot);
  executeQuery4("\"Customer1\"" , 2013, "\"closed\""    , true       );
  executeQuery4("\"Customer3\"" , 2014, "\"deprecated\"", true       );
  executeQuery4("\"Customer11\"", 2013, "\"on hold\""   , false      );
}


#define executeQuery5(id, shouldBeHot) \
{\
  parameter_map_t map;\
  setParameter(map, "id", id);\
\
  const auto file = loadParameterized(QUERY5, map);\
  executeAndWait(file);\
}

TEST_F(AgingTests , Query5) {
//executeQuery5(id, shouldBeHot);
  executeQuery5(1 , true       );
  executeQuery5(17, false      );
}

} } // namespace hyrise::access

