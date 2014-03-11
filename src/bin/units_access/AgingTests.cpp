// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"
#include "helper.h"

#include <io/StorageManager.h>
#include <access/aging/QueryManager.h>

#define QUERYC   "test/aging/inv-qc.json"
#define QUERYS   "test/aging/inv-qs.json"
#define QUERYY   "test/aging/inv-qy.json"
#define QUERYCS  "test/aging/inv-qcs.json"
#define QUERYCY  "test/aging/inv-qcy.json"
#define QUERYSY  "test/aging/inv-qsy.json"
#define QUERYCSY "test/aging/inv-qcsy.json"
#define QUERYId  "test/aging/inv-qid.json"

#define AGING_RUN "test/aging/agingrun.json"
#define LOAD_STATS "test/aging/loadStatistic.json"
#define REGISTER_QUERIES "test/aging/registerQueries.json"

namespace hyrise {
namespace access {

class JsonAgingTests : public Test {
 public:
  virtual void SetUp() {
      const auto file = loadFromFile(REGISTER_QUERIES);
      executeAndWait(file);
  }

  virtual void TearDown() {
    io::StorageManager::getInstance()->clear();
    QueryManager::instance().clear();
  }
};

class JsonAgingTestsAged : public JsonAgingTests {
 public:
  virtual void SetUp() {
    JsonAgingTests::SetUp();
    {
      const auto file = loadFromFile(LOAD_STATS);
      executeAndWait(file);
    }
    {    
      const auto file = loadFromFile(AGING_RUN);
      std::cout << "????????????????????????????????????????????????????????????" << std::endl;
      executeAndWait(file);
      std::cout << "************************************************************" << std::endl;
    }
  }

  virtual void TearDown() {
    JsonAgingTests::TearDown();
  }
};

#define executeJson(query, map) \
  const auto file = loadParameterized(query, map);\
  const auto& result = executeAndWait(file);\
  EXPECT_GE(result->size(), 1);


#define executeQueryC(customer) \
{\
  parameter_map_t map;\
  setParameter(map, "customer", customer);\
  executeJson(QUERYC, map);\
}

#define executeQueryS(status) \
{\
  parameter_map_t map;\
  setParameter(map, "status", status);\
  executeJson(QUERYS, map);\
}

#define executeQueryY(year) \
{\
  parameter_map_t map;\
  setParameter(map, "year", year);\
  executeJson(QUERYY, map);\
}

#define executeQueryId(id) \
{\
  parameter_map_t map;\
  setParameter(map, "id", id);\
  executeJson(QUERYId, map);\
}


#define executeQueryCS(customer, status) \
{\
  parameter_map_t map;\
  setParameter(map, "customer", customer);\
  setParameter(map, "status", status);\
  executeJson(QUERYCS, map);\
}

#define executeQueryCY(customer, year) \
{\
  parameter_map_t map;\
  setParameter(map, "customer", customer);\
  setParameter(map, "year", year);\
  executeJson(QUERYCY, map);\
}

#define executeQuerySY(status, year) \
{\
  parameter_map_t map;\
  setParameter(map, "status", status);\
  setParameter(map, "year", year);\
  executeJson(QUERYSY, map);\
}

#define executeQueryCSY(customer, year, status) \
{\
  parameter_map_t map;\
  setParameter(map, "customer", customer);\
  setParameter(map, "status", status);\
  setParameter(map, "year", year);\
  executeJson(QUERYCSY, map);\
}

TEST_F(JsonAgingTests, QueryC) {
//executeQueryC(customer         );
  executeQueryC("\"Customer1\""  );
  executeQueryC("\"Customer2\""  );
  executeQueryC("\"Customer5\""  );
  executeQueryC("\"Customer15\"" );
  executeQueryC("\"Customer19\"" );
  executeQueryC("\"Customer20\"" );
}

TEST_F(JsonAgingTests, QueryS) {
//executeQueryS(status          );
  executeQueryS("\"on hold\""   );
  executeQueryS("\"in process\"");
  executeQueryS("\"open\""      );
  executeQueryS("\"closed\""    );
  executeQueryS("\"deprecated\"");
}

TEST_F(JsonAgingTests, QueryY) {
//executeQueryY(year);
  executeQueryY(2005);
  executeQueryY(2007);
  executeQueryY(2014);
}

TEST_F(JsonAgingTests, QueryCY) {
//executeQueryCY(customer        , year);
  executeQueryCY("\"Customer3\"" , 2007);
  executeQueryCY("\"Customer9\"" , 2009);
  executeQueryCY("\"Customer15\"", 2012);
  executeQueryCY("\"Customer19\"", 2014);
}

TEST_F(JsonAgingTests, QueryCS) {
//executeQueryCS(customer        , status          );
  executeQueryCS("\"Customer10\"", "\"in process\"");
  executeQueryCS("\"Customer4\"" , "\"closed\""    );
  executeQueryCS("\"Customer20\"", "\"open\""      );
}

TEST_F(JsonAgingTests, QuerySY) {
//executeQuery3(status           , year);
  executeQuerySY("\"in process\"", 2007);
  executeQuerySY("\"closed\""    , 2012);
  executeQuerySY("\"in process\"", 2014);
  executeQuerySY("\"closed\""    , 2006);
  executeQuerySY("\"open\""     , 2013);
}

TEST_F(JsonAgingTests, QueryCSY) {
//executeQueryCSY(customer        , year, status          );
  executeQueryCSY("\"Customer4\"" , 2008, "\"deprecated\"");
  executeQueryCSY("\"Customer9\"" , 2014, "\"on hold\""   );
  executeQueryCSY("\"Customer1\"" , 2005, "\"closed\""    );
  executeQueryCSY("\"Customer12\"", 2011, "\"in process\"");
  executeQueryCSY("\"Customer4\"" , 2007, "\"closed\""    );
}

TEST_F(JsonAgingTests, QueryId) {
//executeQueryId(id);
  executeQueryId(1 );
  executeQueryId(17);
  executeQueryId(41);
}






TEST_F(JsonAgingTestsAged, QueryC) {
//executeQueryC(customer         );
  executeQueryC("\"Customer1\""  );
  executeQueryC("\"Customer2\""  );
  executeQueryC("\"Customer5\""  );
  executeQueryC("\"Customer15\"" );
  executeQueryC("\"Customer19\"" );
  executeQueryC("\"Customer20\"" );
}

TEST_F(JsonAgingTestsAged, QueryS) {
//executeQueryS(status          );
  executeQueryS("\"on hold\""   );
  executeQueryS("\"in process\"");
  executeQueryS("\"open\""      );
  executeQueryS("\"closed\""    );
  executeQueryS("\"deprecated\"");
}

TEST_F(JsonAgingTestsAged, QueryY) {
//executeQueryY(year);
  executeQueryY(2005);
  executeQueryY(2007);
  executeQueryY(2014);
}

TEST_F(JsonAgingTestsAged, QueryCY) {
//executeQueryCY(customer        , year);
  executeQueryCY("\"Customer3\"" , 2007);
  executeQueryCY("\"Customer9\"" , 2009);
  executeQueryCY("\"Customer15\"", 2012);
  executeQueryCY("\"Customer19\"", 2014);
}

TEST_F(JsonAgingTestsAged, QueryCS) {
//executeQueryCS(customer        , status          );
  executeQueryCS("\"Customer10\"", "\"in process\"");
  executeQueryCS("\"Customer4\"" , "\"closed\""    );
  executeQueryCS("\"Customer20\"", "\"open\""      );
}

TEST_F(JsonAgingTestsAged, QuerySY) {
//executeQuery3(status           , year);
  executeQuerySY("\"in process\"", 2007);
  executeQuerySY("\"closed\""    , 2012);
  executeQuerySY("\"in process\"", 2014);
  executeQuerySY("\"closed\""    , 2006);
  executeQuerySY("\"open\""     , 2013);
}

TEST_F(JsonAgingTestsAged, QueryCSY) {
//executeQueryCSY(customer        , year, status          );
  executeQueryCSY("\"Customer4\"" , 2008, "\"deprecated\"");
  executeQueryCSY("\"Customer9\"" , 2014, "\"on hold\""   );
  executeQueryCSY("\"Customer1\"" , 2005, "\"closed\""    );
  executeQueryCSY("\"Customer12\"", 2011, "\"in process\"");
  executeQueryCSY("\"Customer4\"" , 2007, "\"closed\""    );
}

TEST_F(JsonAgingTestsAged, QueryId) {
//executeQueryId(id);
  executeQueryId(1 );
  executeQueryId(17);
  executeQueryId(41);
}

} } // namespace hyrise::access

