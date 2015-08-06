// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/SubString.h"
#include "access/UnionAll.h"
#include "io/shortcuts.h"
#include "testing/test.h"
#include <json.h>

namespace hyrise {
namespace access {

class SubStringTests : public AccessTest {};

TEST_F(SubStringTests, basic_substring_test) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/students.tbl");

  SubString sst;
  sst.addInput(t);

  sst.addField(0);
  sst.addStart(1);
  sst.addCount(2);
  sst.addColName("SUB_NAME");

  sst.addField(2);
  sst.addStart(3);
  sst.addCount(1);
  sst.addColName("SUB_CITY");

  sst.execute();
  const auto& result = sst.getResultTable();

  ASSERT_EQ(59u, result->size());
  ASSERT_EQ("SUB_NAME", result->nameOfColumn(4));
  ASSERT_EQ("SUB_CITY", result->nameOfColumn(5));
  ASSERT_EQ(703567, result->getValue<storage::hyrise_int_t>(1, 0));
  ASSERT_EQ("al", result->getValue<storage::hyrise_string_t>(4, 0));
  ASSERT_EQ("l", result->getValue<storage::hyrise_string_t>(5, 0));
  ASSERT_EQ("uk", result->getValue<storage::hyrise_string_t>(4, 58));
  ASSERT_EQ("l", result->getValue<storage::hyrise_string_t>(5, 58));
}

TEST_F(SubStringTests, border_cases) {
  storage::c_atable_ptr_t t = io::Loader::shortcuts::load("test/students.tbl");

  Json::Value root;
  Json::Reader reader;

  std::string query[11];
  query[0] = "{\"type\": \"SubString\", \"strstart\": [1,1], \"strcount\": [1,1], \"as\": [\"SUBSTR1\",\"SUBSTR2\"]}";

  query[1] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strcount\": [1,1], \"as\": "
      "[\"SUBSTR1\",\"SUBSTR2\"]}";
  query[2] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1], \"strcount\": [1,1], \"as\": "
      "[\"SUBSTR1\",\"SUBSTR2\"]}";
  query[3] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1,1,1], \"strcount\": [1,1], \"as\": "
      "[\"SUBSTR1\",\"SUBSTR2\"]}";

  query[4] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1,1], \"as\": "
      "[\"SUBSTR1\",\"SUBSTR2\"]}";
  query[5] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1,1], \"strcount\": [1], \"as\": "
      "[\"SUBSTR1\",\"SUBSTR2\"]}";
  query[6] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1,1], \"strcount\": [1,1,1], \"as\": "
      "[\"SUBSTR1\",\"SUBSTR2\"]}";

  query[7] = "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1,1], \"strcount\": [1,1]}";
  query[8] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1,1], \"strcount\": [1,1], \"as\": "
      "[\"SUBSTR\"]}";
  query[9] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1,1], \"strcount\": [1,1], \"as\": "
      "[\"SUBSTR1\",\"SUBSTR2\",\"SUBSTR3\"]}";

  query[10] =
      "{\"type\": \"SubString\", \"fields\": [\"name\",\"city\"], \"strstart\": [1,1], \"strcount\": [1,1], \"as\": "
      "[\"SUBSTR1\",\"SUBSTR2\"]}";

  bool parsingSuccessful = false;

  for (int i = 0; i < 10; i++) {
    parsingSuccessful = reader.parse(query[i], root);
    ASSERT_TRUE(parsingSuccessful);
    ASSERT_THROW(QueryParser::instance().parse("SubString", root), std::runtime_error);
  }

  parsingSuccessful = reader.parse(query[10], root);
  ASSERT_TRUE(parsingSuccessful);
  auto ps = std::dynamic_pointer_cast<PlanOperation>(QueryParser::instance().parse("SubString", root));
  ps->addInput(t);
  ps->execute();
  ASSERT_EQ(OpSuccess, ps->getState());
}
}
}
