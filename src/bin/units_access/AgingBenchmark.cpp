// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <cstdlib>
#include <time.h>

#include <set>
#include <iostream>
#include <chrono>

#include <storage/storage_types.h>
#include <storage/TableBuilder.h>
#include <storage/Store.h>

#include <io/StorageManager.h>
#include <access/aging/QueryManager.h>
#include <access/aging/RegisterQuery.h>
#include <access/aging/TableStatistic.h>
#include <access/aging/StatisticSnapshot.h>
#include <access/aging/AgingRun.h>
#include <access/aging/expressions/EqualExpression.h>

#include <testing/TableEqualityTest.h>

namespace hyrise {
namespace access {

template <size_t s,
          size_t hs,
          size_t vc,
          size_t hvc,
          bool rand>
class TestSpec {
 public:
  static size_t size() {return s; }
  static size_t hotSize()  {return hs; }
  static size_t valuec() {return vc; }
  static size_t hotValuec() {return hvc; }
  static bool random() {return rand; }
};

#define FIELD_NAME "field"
#define TABLE_NAME "BENCHMARK_TABLE"
#define QUERY_NAME "Qyery"

template <class spec>
class AgingTests : public Test {
 public:
  AgingTests () {
    if (spec::size() <= spec::hotSize() || spec::hotSize() == 0 || spec::size() < spec::valuec() ||
        spec::hotSize() < spec::hotValuec() || spec::valuec() <= spec::hotValuec() || spec::hotValuec == 0 ||
        spec::hotSize() < spec::hotValuec())
      throw std::runtime_error("senseless configuration");

    srand(time(NULL));

    std::cout << spec::size() << " tuples, " << spec::hotSize() * 100.0f / spec::size() << "% hot, "
              << spec::valuec() << " different values (" << spec::valuec() * 100.0f / spec::size() << "%)" << std::endl;

    storage::TableBuilder::param_list list;
    list.append().set_type("INTEGER").set_name(FIELD_NAME);
    const auto& table = storage::TableBuilder::build(list, false);
    table->resize(spec::size());

    size_t cur = 0;
    //hot values
    for (; cur < spec::hotSize(); ++cur) {
      if (spec::random())
        table->setValue<storage::hyrise_int_t>(0, cur, randomHotValue());
      else
        table->setValue<storage::hyrise_int_t>(0, cur, hotValue(cur));
    }
    //cold values
    for (; cur < spec::size(); ++cur) {
      if (spec::random())
        table->setValue<storage::hyrise_int_t>(0, cur, randomColdValue());
      else
        table->setValue<storage::hyrise_int_t>(0, cur, coldValue(cur));
    }

    auto& sm = *io::StorageManager::getInstance();
    sm.clear();
    _table =  std::make_shared<storage::Store>(table);
    sm.add(TABLE_NAME, _table);
  }

  virtual void SetUp() {}
  virtual void TearDown() {}

  inline storage::hyrise_int_t hotValue(size_t c) {
    return c % spec::hotValuec();
  }
  inline storage::hyrise_int_t coldValue(size_t c) {
    return c % (spec::valuec() - spec::hotValuec()) + spec::hotValuec();
  }
  inline storage::hyrise_int_t value(size_t c) {
    return c % spec::valuec();
  }

  inline storage::hyrise_int_t randomHotValue() {
    return rand() % spec::hotValuec();
  }
  inline storage::hyrise_int_t randomColdValue() {
    return rand() % (spec::valuec() - spec::hotValuec()) + spec::hotValuec();
  }
  inline storage::hyrise_int_t randomValue() {
    return rand() % spec::valuec();
  }

  storage::astat_ptr_t createStatistic() {
    storage::TableBuilder::param_list list;
    list.append().set_type("INTEGER").set_name("value");
    list.append().set_type("INTEGER").set_name(QUERY_NAME);
    const auto& statisticTable = storage::TableBuilder::build(list, false);
    statisticTable->resize(spec::valuec());

    for (size_t row = 0; row < spec::valuec(); ++row) {
      statisticTable->setValue<storage::hyrise_int_t>(0, row, row);
      statisticTable->setValue<storage::hyrise_int_t>(1, row, row < spec::hotValuec());
    }

    auto& sm = *io::StorageManager::getInstance();
    const auto& table = sm.get<storage::AbstractTable>(TABLE_NAME);
    const auto& field = table->numberOfColumn(FIELD_NAME);
    return std::make_shared<TableStatistic>(table, field, statisticTable);
  }

  std::shared_ptr<aging::SelectExpression> selectExpression() {
    const auto& expr = std::make_shared<aging::EqualExpression>(TABLE_NAME, FIELD_NAME);
    expr->verify();
    return expr;
  }

  size_t usExecTime(std::function<void()> func) {
    const auto&  start = std::chrono::steady_clock::now();
    func();
    const auto& end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  }

  float benchmark(std::function<void()> func, std::function<void()> setup = nullptr, size_t min = 5, size_t maxMs = 500) {
    const size_t maxUs = maxMs * 1000;

    u_int64_t totalUs = 0;
    unsigned run = 0;
    for (; run < min; ++run) {
      if (setup) setup();
      totalUs += this->usExecTime(func);
    }
    while (totalUs < maxUs) {
      ++run;
      if (setup) setup();
      totalUs += this->usExecTime(func);
    }

    return totalUs * 0.001 / run;
  }

  storage::atable_ptr_t _table;
};

template <class spec>
class StatisticTests : public AgingTests<spec> {
 public:
  StatisticTests() :
      tStat(this->createStatistic()),
      sStat(std::make_shared<StatisticSnapshot>(*tStat)) {
    auto& qm = QueryManager::instance();
    qm.registerQuery(QUERY_NAME, this->selectExpression());
  }

 public:
  storage::astat_ptr_t tStat;
  storage::astat_ptr_t sStat;
};

template <class spec>
class AgingRunTests : public AgingTests<spec> {
 public:
  AgingRunTests() {
    auto& qm = QueryManager::instance();
    qm.registerQuery(QUERY_NAME, this->selectExpression());
  }

  virtual void SetUp() {
    AgingTests<spec>::SetUp();
    auto& sm = *io::StorageManager::getInstance();
    sm.setStatisticFor(TABLE_NAME, FIELD_NAME, this->createStatistic());
  }

  virtual void TearDown() {
    AgingTests<spec>::TearDown();
    io::StorageManager::getInstance()->clear();
  }
};

  

typedef ::testing::Types<TestSpec<1000    , 1     , 2     , 1     , false>,
                         TestSpec<1000    , 1     , 5     , 1     , false>,
                         TestSpec<1000    , 1     , 10    , 1     , false>,
                         TestSpec<1000    , 1     , 50    , 1     , false>,
                         TestSpec<1000    , 1     , 100   , 1     , false>,
                         TestSpec<1000    , 1     , 500   , 1     , false>,
                         TestSpec<1000    , 1     , 1000  , 1     , false>/*,

                         TestSpec<1000    , 10    , 2     , 1     , false>,
                         TestSpec<1000    , 10    , 5     , 1     , false>,
                         TestSpec<1000    , 10    , 50    , 1     , false>,
                         TestSpec<1000    , 10    , 100   , 1     , false>,
                         TestSpec<1000    , 10    , 1000  , 1     , false>,

                         TestSpec<25      , 1     , 2     , 2     , false>,
                         TestSpec<25      , 2     , 10    , 2     , false>,
                         TestSpec<25      , 2     , 10    , 2     , false>,
                         TestSpec<100     , 5     , 20    , 2     , false>*/>
        MySmallTypes;

TYPED_TEST_CASE(AgingTests, MySmallTypes);
TYPED_TEST_CASE(AgingRunTests, MySmallTypes);
TYPED_TEST_CASE(StatisticTests, MySmallTypes);



TYPED_TEST(StatisticTests, sameReference) {
  EXPECT_EQ(this->tStat->table(), this->sStat->table());
  EXPECT_EQ(this->tStat->field(), this->sStat->field());
}

TYPED_TEST(StatisticTests, sameQueries) {
  auto queries1 = this->tStat->queries();
  auto queries2 = this->sStat->queries();

  ASSERT_EQ(queries1.size(), queries2.size());
  
  std::sort(queries1.begin(), queries1.end());
  std::sort(queries2.begin(), queries2.end());

  for (size_t i = 0; i < queries1.size(); ++i)
   ASSERT_EQ(queries1.at(i), queries2.at(i));
}

TYPED_TEST(StatisticTests, sameVids) {
  const auto& vids1 = this->tStat->vids();
  const auto& vids2 = this->sStat->vids();

  ASSERT_LE(vids1.size(), vids2.size());
  
  std::set<storage::value_id_t> set1(vids1.begin(), vids1.end());
  std::set<storage::value_id_t> set2(vids2.begin(), vids2.end());

  //only one direction
  for (const auto& vid : set2)
   ASSERT_TRUE(std::find(set1.begin(), set1.end(), vid) != set1.end());
}

TYPED_TEST(StatisticTests, vidsUnique) {
  const auto& vids = this->sStat->vids();
  std::set<storage::value_id_t> set(vids.begin(), vids.end());

  ASSERT_EQ(vids.size(), set.size());
}

TYPED_TEST(StatisticTests, completeWalkthrough) {
  const auto& queries = this->sStat->queries();
  const auto& other = this->sStat;
  for (const auto& query : queries) {
    ASSERT_TRUE(this->tStat->isQueryRegistered(query));
    ASSERT_TRUE(this->sStat->isQueryRegistered(query));
    this->sStat->valuesDo(query, [&query, &other](storage::value_id_t vid, bool hot){
                                   if (!other->isVidRegistered(vid) || other->isHot(query, vid) != hot)
                                     throw std::runtime_error("failed");
                                 });
  }
}



TYPED_TEST(AgingRunTests, QualitativeTest) {
  auto& sm = *io::StorageManager::getInstance();
  {
    const auto& prevTable = sm.get<storage::AbstractTable>(TABLE_NAME);

    AgingRun ar(TABLE_NAME);
    ar.execute();

    const auto& newTable = sm.get<storage::AbstractTable>(TABLE_NAME);
    EXPECT_NE(prevTable, newTable);
    EXPECT_RELATION_EQ(prevTable, newTable);
  }
  {
    const auto& prevTable = sm.get<storage::AbstractTable>(TABLE_NAME);

    AgingRun ar(TABLE_NAME);
    ar.execute();

    const auto& newTable = sm.get<storage::AbstractTable>(TABLE_NAME);
    EXPECT_EQ(prevTable, newTable); //this time equal
    EXPECT_RELATION_EQ(prevTable, newTable);
  }
}


TYPED_TEST(AgingRunTests, Benchmark1st) {
  auto& sm = *io::StorageManager::getInstance();
  std::shared_ptr<AgingRun> ar;

  const auto& setup = [&ar, &sm, this]() { ar = std::make_shared<AgingRun>(TABLE_NAME);
                                           /*sm.replace(TABLE_NAME, this->_table);*/ };
  const auto& exec = [&ar]() { ar->execute(); };

  std::cout << this->benchmark(exec, setup) << "ms / AgingRun" << std::endl;
}

TYPED_TEST(AgingRunTests, Benchmark2st) {
  auto& sm = *io::StorageManager::getInstance();
  std::shared_ptr<AgingRun> ar;

  const auto& setup = [&ar, &sm, this]() { ar = std::make_shared<AgingRun>(TABLE_NAME);
                                           /*sm.replace(TABLE_NAME, this->_table);
                                           AgingRun ar2(TABLE_NAME);
                                           ar2.execute();*/ };
  const auto& exec = [&ar]() { ar->execute(); };

  std::cout << this->benchmark(exec, setup) << "ms / AgingRun" << std::endl;
}

} } // namespace hyrise::access

