// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include <string>
#include <iostream>
#include <vector>

#include "helper.h"

#include "layouter/base.h"
#include "layouter/matrix.h"
#include "layouter/incremental.h"

#include "io/shortcuts.h"

#include "boost/assign/list_of.hpp"
#include "boost/assign/std/vector.hpp"
#include "boost/assign.hpp"

#include "gtest/gtest.h"

using namespace boost::assign;

namespace hyrise {
namespace layouter {

class LayouterTests : public ::testing::Test {

 public:

  Schema simpleSchema() {
    std::vector< std::string > names;
    names.push_back("A");
    names.push_back("B");
    names.push_back("C");

    std::vector<unsigned> atts;
    atts.push_back(4);
    atts.push_back(4);
    atts.push_back(4);

    Schema s(atts, 1000000, names);
    return s;
  }

  Schema wideSchema() {
    std::vector< std::string > names;
    names.push_back("A");
    names.push_back("B");
    names.push_back("C");
    names.push_back("D");
    names.push_back("E");
    names.push_back("F");
    names.push_back("G");
    names.push_back("H");

    std::vector<unsigned> atts;
    atts.push_back(4);
    atts.push_back(4);
    atts.push_back(4);
    atts.push_back(4);
    atts.push_back(4);
    atts.push_back(4);
    atts.push_back(4);
    atts.push_back(4);


    Schema s(atts, 100000, names);
    return s;
  }

  Result executeSimpleQuery() {

    Schema s = simpleSchema();

    std::vector<unsigned> aq1;
    aq1.push_back(0);

    // type, attributes, selection, weight
    Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
    s.add(&q1);

    BaseLayouter bl;
    bl.layout(s, HYRISE_COST);

    // for (const auto& r: bl.getNBestResults(9999)) {
    //   r.print();
    // }

    return bl.getBestResult();
  }

};


TEST_F(LayouterTests, test_subset_ordering) {

  subset_t a, b, c;
  std::vector<subset_t> all;

  a += 1, 2, 3;
  b += 3, 3;
  c += 3, 1;

  all += a, b, c;
  std::sort(all.begin(), all.end(), subset_t_lt);

  ASSERT_EQ(c, all[0]);
  ASSERT_EQ(b, all[1]);
  ASSERT_EQ(a, all[2]);

}


TEST_F(LayouterTests, selection_experiment_for_thesis) {

  for (size_t numAttrs = 1; numAttrs <= 100; ++numAttrs) {

    std::vector<std::string> names;
    std::vector<unsigned> atts;
    std::vector<unsigned>  pq;
    std::vector<unsigned>  sq;


    for (size_t i = 0; i < numAttrs; ++i) {
      names += "COL" + std::to_string(i);
      atts += 4;
      sq += i;
    }

    Schema s(atts, 10000000, names);


    // Add projection
    pq += 0;
    Query *q1 = new Query(LayouterConfiguration::access_type_fullprojection, pq, -1.0, 1);
    s.add(q1);


    // Add selection
    Query *q2 = new Query(LayouterConfiguration::access_type_outoforder, sq, 0.1, 1);
    s.add(q2);

    // Check the cost
    // std::cout << numAttrs << " " <<  q1->containerCost(sq, s, HYRISE_COST) << " " << q2->containerCost(sq, s, HYRISE_COST) << std::endl;
  }

}

TEST_F(LayouterTests, cost_calculation_with_att_order) {
  std::vector< std::string > names;
  names.push_back("ID");
  names.push_back("NAME");
  names.push_back("MAIL");
  names.push_back("COMPANY");
  names.push_back("PHONE");
  names.push_back("ORG");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000000, names);

  std::vector<unsigned> aq2;
  aq2.push_back(1);
  aq2.push_back(2);
  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.02, 1);
  s.add(&q2);


  std::vector<unsigned> part;
  part.push_back(1);
  part.push_back(2);
  part.push_back(3);
  //part.push_back(1);
  ASSERT_NEAR(19555.2, q2.containerCost(part, s, HYRISE_COST), 0.02);

}


TEST_F(LayouterTests, DISABLED_correct_cost_calculation) {
  std::vector< std::string > names;
  names.push_back("ID");
  names.push_back("NAME");
  names.push_back("MAIL");
  names.push_back("COMPANY");
  names.push_back("PHONE");
  names.push_back("ORG");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000000, names);

  // Add first query
  std::vector<unsigned> aq1;
  aq1.push_back(0);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 2);
  s.add(&q1);

  //Add second query
  std::vector<unsigned> aq2;
  aq2.push_back(1);
  aq2.push_back(2);
  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.02, 1);
  s.add(&q2);


  //Add second query
  std::vector<unsigned> aq3;
  aq3.push_back(1);
  aq3.push_back(3);
  Query q3(LayouterConfiguration::access_type_outoforder, aq3, 0.02, 1);
  s.add(&q3);


  // Check the cost
  subset_t first;
  first.push_back(4);

  ASSERT_EQ(0, s.costForSubset(first, HYRISE_COST));

  // Check cost for different partitions
  subset_t second;
  second.push_back(1);
  second.push_back(2);
  second.push_back(3);
  second.push_back(4);

  ASSERT_EQ(19555.2, q2.containerCost(second, s, HYRISE_COST));
}


TEST_F(LayouterTests, divide_and_conquer_affinity) {
  std::vector<unsigned> atts;
  std::vector<std::string> names;

  for (int i = 0; i < 10; ++i) {
    std::stringstream a;
    a << "attr_" << i;
    atts.push_back(4);
    names.push_back(a.str());
  }

  Schema *s = new Schema(atts, 1000000, names);

  std::vector<unsigned> aq;
  aq += 0, 1, 2;

  Query *q;
  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);

  aq = std::vector<unsigned>();
  aq += 0, 1;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);

  aq = std::vector<unsigned>();
  aq += 0, 1;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);

  aq = std::vector<unsigned>();
  aq += 2, 3;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);

  aq = std::vector<unsigned>();
  aq += 3, 4;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);

  aq = std::vector<unsigned>();
  aq += 5, 6, 7, 8;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);

  aq = std::vector<unsigned>();
  aq += 5, 7, 8, 9;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);


  aq = std::vector<unsigned>();
  aq += 5, 7, 9;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);

  aq = std::vector<unsigned>();
  aq += 3, 9;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);

  aq = std::vector<unsigned>();
  aq += 0, 1, 9;

  q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 2);
  s->add(q);



  BaseLayouter *l = new DivideAndConquerLayouter();
  l->layout(*s, HYRISE_COST);

  //l->getBestResult().print();
}

TEST_F(LayouterTests, divide_and_conquer_with_larger_group) {
  std::vector<unsigned> atts;
  std::vector<std::string> names;

  for (int i = 0; i < 10; ++i) {
    std::stringstream a;
    a << "attr_" << i;
    atts.push_back(4);
    names.push_back(a.str());
  }

  Schema *s = new Schema(atts, 1000000, names);

  // Add the queries
  for (int i = 0; i < 10; ++i) {
    std::vector<unsigned> aq;
    for (int j = 0; j < i; ++j)
      aq.push_back(j);

    Query *q = new Query(LayouterConfiguration::access_type_fullprojection, aq, -1.0, 1);
    s->add(q);
  }

  BaseLayouter *l = new DivideAndConquerLayouter();
  l->layout(*s, HYRISE_COST);
}

TEST_F(LayouterTests, candidate_layouter_only_one_primary_partition) {
  std::vector< std::string > names;
  names.push_back("ID");
  names.push_back("NAME");
  names.push_back("MAIL");
  names.push_back("COMPANY");
  names.push_back("PHONE");
  names.push_back("ORG");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000000, names);

  std::vector<unsigned> aq1;
  aq1.push_back(0);
  aq1.push_back(1);
  aq1.push_back(2);
  aq1.push_back(3);
  aq1.push_back(4);
  aq1.push_back(5);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);

  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);

  // ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());

  std::vector<Result> results = cl.getNBestResults(9999);
  ASSERT_LT(0u, results.size()) << "Even with one primary partition there should be a result";

}

TEST_F(LayouterTests, DISABLED_candidate_test_from_presentation) {
  std::vector< std::string > names;
  names.push_back("ID");
  names.push_back("NAME");
  names.push_back("MAIL");
  names.push_back("COMPANY");
  names.push_back("PHONE");
  names.push_back("ORG");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000000, names);

  std::vector<unsigned> aq1;
  aq1.push_back(5);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);

  std::vector<unsigned> aq2;
  aq2.push_back(0);
  aq2.push_back(1);

  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.1, 1);
  s.add(&q2);

  std::vector<unsigned> aq3;
  aq3.push_back(0);
  aq3.push_back(3);

  Query q3(LayouterConfiguration::access_type_outoforder, aq3, 0.1, 1);
  s.add(&q3);

  std::vector<unsigned> aq4;
  aq4.push_back(5);

  Query q4(LayouterConfiguration::access_type_fullprojection, aq4, -1.0, 1);
  s.add(&q4);

  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  BaseLayouter bl;
  bl.layout(s, HYRISE_COST);

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());

  ASSERT_DOUBLE_EQ(cl.getBestResult().totalCost, bl.getBestResult().totalCost);
}

TEST_F(LayouterTests, matrix_metis_test) {
  Matrix<int> m(3);

  m.set(0, 1, 99).set(1, 0, 99);
  m.set(0, 2, 42).set(2, 0, 42);

  ASSERT_EQ(2, m.numEdges());
  ASSERT_EQ(3, m.numVertices());

  adj_t t = m.buildAdjacency();
  ASSERT_EQ(0, t.xadj[0]);
  ASSERT_EQ(2, t.xadj[1]);
  ASSERT_EQ(3, t.xadj[2]);
  ASSERT_EQ(4, t.xadj[3]);

  ASSERT_EQ(1, t.adjncy[0]);
  ASSERT_EQ(2, t.adjncy[1]);
  ASSERT_EQ(0, t.adjncy[2]);
  ASSERT_EQ(0, t.adjncy[3]);

  // Check the weights
  ASSERT_EQ(99, t.adjwgt[0]);
  ASSERT_EQ(42, t.adjwgt[1]);
  ASSERT_EQ(99, t.adjwgt[2]);
  ASSERT_EQ(42, t.adjwgt[3]);

}


TEST_F(LayouterTests, simple_divide_and_conquer) {
  Schema s = wideSchema();

  // type, attributes, selection, weight
  std::vector<unsigned> aq1;
  aq1.push_back(0);
  aq1.push_back(1);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);

  std::vector<unsigned> aq2;
  aq2.push_back(1);
  aq2.push_back(2);

  Query q2(LayouterConfiguration::access_type_fullprojection, aq2, -1.0, 1);
  s.add(&q2);

  std::vector<unsigned> aq3;
  aq3.push_back(7);
  aq3.push_back(6);

  Query q3(LayouterConfiguration::access_type_outoforder, aq3, 0.1, 1);
  s.add(&q3);


  DivideAndConquerLayouter bl;
  bl.layout(s, HYRISE_COST);
  //std::cout << bl.getColumnCost() << " " << bl.getRowCost() << std::endl;


  Result r = bl.getBestResult();
  double tmp =  r.totalCost;

  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());


  ASSERT_DOUBLE_EQ(tmp, r.totalCost);


}

TEST_F(LayouterTests, layout_print_full_names) {
  Result r = executeSimpleQuery();

  std::string tmp = r.output();
  std::string cmp = loadFromFile("test/header/layouter_simple.tbl");
  ASSERT_EQ(cmp, tmp);
}

TEST_F(LayouterTests, layout_load_table_from_output) {
  Result r = executeSimpleQuery();
  std::string tmp = r.output();

  auto t = io::Loader::shortcuts::loadWithStringHeader("test/tables/only_data.tbl", tmp);
  auto res = io::Loader::shortcuts::loadWithHeader("test/tables/only_data.tbl", "test/header/layouter_simple.tbl");

  ASSERT_TABLE_EQUAL(t, res);
}



TEST_F(LayouterTests, initial_layouter_test) {
  std::vector< std::string > names;
  names.push_back("A");
  names.push_back("B");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000, names);


  std::vector<unsigned> aq1;
  aq1.push_back(0);

  // type, attributes, selection, weight
  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);

  BaseLayouter bl;
  bl.layout(s, HYRISE_COST);

  Result r = bl.getBestResult();
  ASSERT_EQ(r.layout.containerCount(), 2u);
}

TEST_F(LayouterTests, more_complex_layouter_test) {
  std::vector< std::string > names;
  names.push_back("A");
  names.push_back("B");
  names.push_back("C");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 100000, names);


  std::vector<unsigned> aq1;
  aq1.push_back(0);
  //aq1.push_back(1);

  // type, attributes, selection, weight
  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);

  BaseLayouter bl;
  bl.layout(s, HYRISE_COST);

  Result r = bl.getBestResult();
  ASSERT_EQ(r.layout.containerCount(), 2u);
}

TEST_F(LayouterTests, layouter_two_queries) {
  std::vector< std::string > names;
  names.push_back("A");
  names.push_back("B");
  names.push_back("C");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000, names);


  std::vector<unsigned> aq1;
  aq1.push_back(0);

  // type, attributes, selection, weight
  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);


  std::vector<unsigned> aq2;
  aq2.push_back(0);
  aq2.push_back(1);

  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.9, 1);
  s.add(&q2);

  BaseLayouter bl;
  bl.layout(s, HYRISE_COST);

  Result r = bl.getBestResult();
  ASSERT_EQ(r.layout.containerCount(), 3u);
}


TEST_F(LayouterTests, initial_layouter_test_candidate) {
  std::vector< std::string > names;
  names.push_back("A");
  names.push_back("B");
  //names.push_back("C");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  //atts.push_back(4);

  Schema s(atts, 1000, names);


  std::vector<unsigned> aq1;
  aq1.push_back(0);
  //aq1.push_back(1);

  // type, attributes, selection, weight
  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);

  CandidateLayouter bl;
  bl.layout(s, HYRISE_COST);

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(bl.getBestResult() == fcl.getBestResult());


  Result r = bl.getBestResult();
  //r.print();
  ASSERT_EQ(r.layout.containerCount(), 2u);


}

TEST_F(LayouterTests, layouter_two_queries_candidate) {
  std::vector< std::string > names;
  names.push_back("A");
  names.push_back("B");
  names.push_back("C");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000, names);


  std::vector<unsigned> aq1;
  aq1.push_back(0);

  // type, attributes, selection, weight
  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);


  std::vector<unsigned> aq2;
  aq2.push_back(0);
  aq2.push_back(1);

  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.9, 1);
  s.add(&q2);

  CandidateLayouter bl;
  bl.layout(s, HYRISE_COST);

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(bl.getBestResult() == fcl.getBestResult());


  Result r = bl.getBestResult();
  //r.print();
  ASSERT_EQ(r.layout.containerCount(), 3u);

}

TEST_F(LayouterTests, incremental_layouter_initial) {
  std::vector< std::string > names;
  names.push_back("ID");
  names.push_back("NAME");
  names.push_back("MAIL");
  names.push_back("COMPANY");
  names.push_back("PHONE");
  names.push_back("ORG");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000000, names);

  // Add first query
  std::vector<unsigned> aq1;
  aq1.push_back(0);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);

  IncrementalCandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  Result r = cl.getBestResult();

  //Add second query
  std::vector<unsigned> aq2;
  aq2.push_back(0);
  aq2.push_back(1);
  aq2.push_back(2);
  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.1, 1);


  // Incremental layout
  cl.incrementalLayout(&q2);
  r = cl.getBestResult();

  ASSERT_EQ(3u, r.layout.containerCount());

}


TEST_F(LayouterTests, candidate_layoute_merged) {
  std::vector< std::string > names;
  names.push_back("ID");
  names.push_back("NAME");
  names.push_back("MAIL");
  names.push_back("COMPANY");
  names.push_back("PHONE");
  names.push_back("ORG");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000000, names);

  // Add first query
  std::vector<unsigned> aq1;
  aq1.push_back(0);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 2);
  s.add(&q1);

  //Add second query
  std::vector<unsigned> aq2;
  aq2.push_back(0);
  aq2.push_back(1);
  aq2.push_back(2);
  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.01, 1);
  s.add(&q2);

  std::vector<unsigned> aq3;
  aq3.push_back(0);
  aq3.push_back(1);
  aq3.push_back(3);
  Query q3(LayouterConfiguration::access_type_outoforder, aq3, 0.01, 1);
  s.add(&q3);


  std::vector<unsigned> aq4;
  aq4.push_back(0);
  aq4.push_back(1);
  aq4.push_back(4);
  aq4.push_back(5);
  Query q4(LayouterConfiguration::access_type_outoforder, aq4, 0.01, 1);
  s.add(&q4);


  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);



  Result r = cl.getBestResult();
  // r.print();

  //ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());
}

TEST_F(LayouterTests, incrementalLayout_layoute_merged) {
  std::vector< std::string > names;
  names.push_back("ID");
  names.push_back("NAME");
  names.push_back("MAIL");
  names.push_back("COMPANY");
  names.push_back("PHONE");
  names.push_back("ORG");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000000, names);
  Schema sinc(atts, 1000000, names);

  // Add first query
  std::vector<unsigned> aq1;
  aq1.push_back(0);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 2);
  s.add(&q1);
  sinc.add(&q1);

  //Add second query
  std::vector<unsigned> aq2;
  //aq2.push_back(0);
  aq2.push_back(1);
  aq2.push_back(2);
  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.02, 1);
  s.add(&q2);
  sinc.add(&q2);

  std::vector<unsigned> aq3;
  //aq3.push_back(0);
  aq3.push_back(1);
  aq3.push_back(3);
  Query q3(LayouterConfiguration::access_type_outoforder, aq3, 0.02, 1);
  s.add(&q3);


  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);
  Result r = cl.getBestResult();

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());


  IncrementalCandidateLayouter il;
  il.layout(sinc, HYRISE_COST);

  // // Incrementally add
  il.incrementalLayout(&q3);
  Result r2 = il.getBestResult();

  // r.print();
  // r2.print();


  ASSERT_TRUE(r == r2);
}

TEST_F(LayouterTests, incrementalLayout_layoute_merged_merge_groups) {
  std::vector< std::string > names;
  names.push_back("ID");
  names.push_back("NAME");
  names.push_back("MAIL");
  names.push_back("COMPANY");
  names.push_back("PHONE");
  names.push_back("ORG");

  std::vector<unsigned> atts;
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);
  atts.push_back(4);

  Schema s(atts, 1000000, names);
  Schema sinc(atts, 1000000, names);

  // Add first query
  std::vector<unsigned> aq1;
  aq1.push_back(0);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 2);
  s.add(&q1);
  sinc.add(&q1);

  //Add second query
  std::vector<unsigned> aq2;
  //aq2.push_back(0);
  aq2.push_back(1);
  aq2.push_back(2);
  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.1, 1);
  s.add(&q2);
  sinc.add(&q2);

  std::vector<unsigned> aq3;
  //aq3.push_back(0);
  aq3.push_back(1);
  aq3.push_back(3);
  Query q3(LayouterConfiguration::access_type_outoforder, aq3, 0.1, 1);
  s.add(&q3);

  std::vector<unsigned> aq4;
  //aq3.push_back(0);
  aq4.push_back(2);
  aq4.push_back(3);
  aq4.push_back(4);
  Query q4(LayouterConfiguration::access_type_outoforder, aq4, 0.3, 1);
  s.add(&q4);


  std::vector<unsigned> aq5;
  aq5.push_back(4);
  Query q5(LayouterConfiguration::access_type_fullprojection, aq5, -1, 1);
  //s.add(&q5);



  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);
  Result r = cl.getBestResult();
  // r.print();

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());


  IncrementalCandidateLayouter il;
  il.layout(sinc, HYRISE_COST);

  // Incrementally add
  il.incrementalLayout(&q3);
  il.incrementalLayout(&q4);
  //il.incrementalLayout(&q5);
  Result r2 = il.getBestResult();
  // r2.print();

  ASSERT_TRUE(r == r2);
}

TEST_F(LayouterTests, paper_layouter_performance) {
  std::vector< std::string > names;
  names += "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9";

  std::vector<unsigned> atts;
  atts += 4, repeat(8, 4);

  Schema s(atts, 1000000, names);
  Schema sinc(atts, 1000000, names);

  // Add first query
  std::vector<unsigned> aq1;
  aq1 += 0, 1;

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 1);
  s.add(&q1);
  sinc.add(&q1);

  //Add second query
  std::vector<unsigned> aq2;
  aq2 += 2, 3, 4;

  Query q2(LayouterConfiguration::access_type_fullprojection, aq2, -1.0, 1);
  s.add(&q2);
  sinc.add(&q2);

  // Add third query
  std::vector<unsigned> aq3;
  aq3 += 5;

  Query q3(LayouterConfiguration::access_type_fullprojection, aq3, -1.0, 1);
  s.add(&q3);
  sinc.add(&q3);

  // Add fourth query
  std::vector<unsigned> aq4;
  aq4 += 6, 7, 8;

  Query q4(LayouterConfiguration::access_type_fullprojection, aq4, -1.0, 1);
  s.add(&q4);
  sinc.add(&q4);


  // New Query
  std::vector<unsigned> aq5;
  aq5 += 3, 5;

  Query q5(LayouterConfiguration::access_type_outoforder, aq5, 0.1, 1);
  s.add(&q5);

  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  Result r = cl.getBestResult();

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());

  IncrementalCandidateLayouter il;
  il.layout(sinc, HYRISE_COST);
  il.incrementalLayout(&q5);

  Result r2 = il.getBestResult();

  // r.print();
  // r2.print();


  ASSERT_TRUE(r == r2);
}


TEST_F(LayouterTests, group_merge_performance) {

  std::vector< std::string > names;
  names += "ID", "NAME", "MAIL", "COMPANY", "PHONE", "ORG";

  std::vector<unsigned> atts;
  atts += 4, repeat(5, 4);

  Schema s(atts, 1000000, names);

  // Add first query
  std::vector<unsigned> aq1;
  aq1.push_back(0);

  Query q1(LayouterConfiguration::access_type_fullprojection, aq1, -1.0, 2);
  s.add(&q1);

  //Add second query
  std::vector<unsigned> aq2;
  aq2.push_back(0);
  aq2.push_back(1);
  aq2.push_back(2);
  Query q2(LayouterConfiguration::access_type_outoforder, aq2, 0.01, 1);
  s.add(&q2);

  std::vector<unsigned> aq3;
  aq3.push_back(0);
  aq3.push_back(1);
  aq3.push_back(3);
  Query q3(LayouterConfiguration::access_type_outoforder, aq3, 0.01, 1);
  s.add(&q3);


  std::vector<unsigned> aq4;
  aq4.push_back(0);
  aq4.push_back(1);
  aq4.push_back(4);
  aq4.push_back(5);
  Query q4(LayouterConfiguration::access_type_outoforder, aq4, 0.01, 1);
  s.add(&q4);


  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());


  Result r = cl.getBestResult();
  //r.print();

  // Primary Partitions
  subset_t p1;
  p1 += 0;

  subset_t p13;
  p13 += 0, 2;


  subset_t p2;
  p2 += 1;

  subset_t p3;
  p3 += 2;

  subset_t p23;
  p23 += 1, 2;


  subset_t p4;
  p4 += 3;

  subset_t p234;
  p234 += 1, 2, 3;

  subset_t p5;
  p5 += 4, 5;

  subset_t p2345;
  p2345 += 1, 2, 3, 4, 5;

  subset_t merged;
  merged += 4, 5, 1, 3, 2;


  // std::cout << "p1 " << s.costForSubset(p1, HYRISE_COST) << std::endl;
  // std::cout << "p13 " << s.costForSubset(p13, HYRISE_COST) << std::endl;

  // std::cout << "p2 "  << s.costForSubset(p2, HYRISE_COST) << std::endl;
  // std::cout << "p3 "  << s.costForSubset(p3, HYRISE_COST) << std::endl;

  // std::cout << "p2,p3 "  << s.costForSubset(p23, HYRISE_COST) << std::endl;
  // std::cout << "p2,p3,p4 "  << s.costForSubset(p234, HYRISE_COST) << std::endl;
  // std::cout << "p2,p3,p4,p5 "  << s.costForSubset(p2345, HYRISE_COST) << std::endl;

  // std::cout << "p4 "  << s.costForSubset(p4, HYRISE_COST) << std::endl;
  // std::cout << "p5 "  << s.costForSubset(p5, HYRISE_COST) << std::endl;
  // std::cout << "merged "  << s.costForSubset(merged, HYRISE_COST) << std::endl;


}


TEST_F(LayouterTests , cost_distribution_test) {
  std::vector< std::string > names;
  names += "a", "b", "c", "d", "e", "f", "g";

  std::vector<unsigned> atts;
  atts += 4, repeat(6, 4);

  Schema s(atts, 1000000, names);

  subset_t a1;
  a1 += 2;
  Query q1(LayouterConfiguration::access_type_fullprojection, a1, -1.0, 1);
  s.add(&q1);


  subset_t a2;
  a2 += 0, 1, 2;
  Query q2(LayouterConfiguration::access_type_outoforder, a2, 0.3, 1);
  s.add(&q2);

  subset_t a3;
  a3 += 3, 4;
  Query q3(LayouterConfiguration::access_type_outoforder, a3, 0.3, 1);
  s.add(&q3);

  subset_t a4;
  a4 += 0, 1, 3, 4, 5, 6;
  Query q4(LayouterConfiguration::access_type_outoforder, a4, 0.3, 1);
  s.add(&q4);


  subset_t a5;
  a5 += 2, 3, 4;
  Query q5(LayouterConfiguration::access_type_outoforder, a5, 0.3, 65);
  s.add(&q5);

  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  Result r = cl.getBestResult();
  // r.print();

}


TEST_F(LayouterTests, candidate_generation_performance) {
  std::vector< std::string > names;
  names += "1", "2", "3", "4", "5", "6", "7", "8", "9";

  std::vector<unsigned> atts;
  atts += 4, repeat(8, 4);

  Schema s(atts, 1000000, names);

  subset_t a1 = subset_t(1, 0);
  Query q1(LayouterConfiguration::access_type_fullprojection, a1, -1.0, 2);
  s.add(&q1);

  subset_t a2 = subset_t(1, 1);
  Query q2(LayouterConfiguration::access_type_fullprojection, a2, -1.0, 2);
  s.add(&q2);

  subset_t a3 = subset_t(1, 2);
  Query q3(LayouterConfiguration::access_type_fullprojection, a3, -1.0, 2);
  s.add(&q3);

  subset_t a4 = subset_t(1, 3);
  Query q4(LayouterConfiguration::access_type_fullprojection, a4, -1.0, 2);
  s.add(&q4);

  subset_t a5 = subset_t(1, 4);
  Query q5(LayouterConfiguration::access_type_fullprojection, a5, -1.0, 2);
  s.add(&q5);

  subset_t a6 = subset_t(1, 5);
  Query q6(LayouterConfiguration::access_type_fullprojection, a6, -1.0, 2);
  s.add(&q6);

  subset_t a7 = subset_t(1, 6);
  Query q7(LayouterConfiguration::access_type_fullprojection, a7, -1.0, 2);
  s.add(&q7);

  subset_t a8 = subset_t(1, 7);
  Query q8(LayouterConfiguration::access_type_fullprojection, a8, -1.0, 2);
  //s.add(&q8);

  subset_t a9 = subset_t(1, 8);
  Query q9(LayouterConfiguration::access_type_fullprojection, a9, -1.0, 2);
  //s.add(&q9);

  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);

  // FastCandidateLayouter fcl;
  // fcl.layout(s, HYRISE_COST);
  // ASSERT_TRUE(cl.getBestResult() == fcl.getBestResult());


  Result r = cl.getBestResult();
  // r.print();


}

TEST_F(LayouterTests, candidate_iteration_performance) {
#ifdef EXPENSIVE_TESTS
  std::vector< std::string > names;
  names += "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23";

  std::vector<unsigned> atts;
  atts += 4, repeat(22, 4);

  Schema s(atts, 1000000, names);

  subset_t a1 = subset_t(1, 0);
  Query q1(LayouterConfiguration::access_type_fullprojection, a1, -1.0, 2);
  s.add(&q1);

  subset_t a2 = subset_t(1, 1);
  Query q2(LayouterConfiguration::access_type_fullprojection, a2, -1.0, 2);
  s.add(&q2);

  subset_t a3 = subset_t(1, 2);
  Query q3(LayouterConfiguration::access_type_fullprojection, a3, -1.0, 2);
  s.add(&q3);

  subset_t a4 = subset_t(1, 3);
  Query q4(LayouterConfiguration::access_type_fullprojection, a4, -1.0, 2);
  s.add(&q4);

  subset_t a5 = subset_t(1, 4);
  Query q5(LayouterConfiguration::access_type_fullprojection, a5, -1.0, 2);
  s.add(&q5);

  subset_t a6 = subset_t(1, 5);
  Query q6(LayouterConfiguration::access_type_fullprojection, a6, -1.0, 2);
  s.add(&q6);

  subset_t a7 = subset_t(1, 6);
  Query q7(LayouterConfiguration::access_type_fullprojection, a7, -1.0, 2);
  s.add(&q7);

  subset_t a8 = subset_t(1, 7);
  Query q8(LayouterConfiguration::access_type_fullprojection, a8, -1.0, 2);
  s.add(&q8);

  subset_t a9 = subset_t(1, 8);
  Query q9(LayouterConfiguration::access_type_fullprojection, a9, -1.0, 2);
  s.add(&q9);

  subset_t a10 = subset_t(1, 9);
  Query q10(LayouterConfiguration::access_type_fullprojection, a10, -1.0, 2);
  s.add(&q10);

  subset_t a11 = subset_t(1, 10);
  Query q11(LayouterConfiguration::access_type_fullprojection, a11, -1.0, 2);
  s.add(&q11);

  subset_t a12 = subset_t(1, 11);
  Query q12(LayouterConfiguration::access_type_fullprojection, a12, -1.0, 2);
  s.add(&q12);

  subset_t a13 = subset_t(1, 12);
  Query q13(LayouterConfiguration::access_type_fullprojection, a13, -1.0, 2);
  s.add(&q13);

  subset_t a14 = subset_t(1, 13);
  Query q14(LayouterConfiguration::access_type_fullprojection, a14, -1.0, 2);
  s.add(&q14);

  subset_t a15 = subset_t(1, 14);
  Query q15(LayouterConfiguration::access_type_fullprojection, a15, -1.0, 2);
  s.add(&q15);

  subset_t a16 = subset_t(1, 15);
  Query q16(LayouterConfiguration::access_type_fullprojection, a16, -1.0, 2);
  s.add(&q16);

  subset_t a17 = subset_t(1, 16);
  Query q17(LayouterConfiguration::access_type_fullprojection, a17, -1.0, 2);
  s.add(&q17);

  subset_t a18 = subset_t(1, 17);
  Query q18(LayouterConfiguration::access_type_fullprojection, a18, -1.0, 2);
  //s.add(&q18);

  subset_t a19 = subset_t(1, 18);
  Query q19(LayouterConfiguration::access_type_fullprojection, a19, -1.0, 2);
  //s.add(&q19);

  subset_t a20 = subset_t(1, 19);
  Query q20(LayouterConfiguration::access_type_fullprojection, a20, -1.0, 2);
  //s.add(&q20);

  subset_t a21 = subset_t(1, 20);
  Query q21(LayouterConfiguration::access_type_fullprojection, a21, -1.0, 2);
  //s.add(&q21);

  CandidateLayouter cl;
  cl.layout(s, HYRISE_COST);
#endif
}



// This is some kind of performance regression that should only be build in produciton mode

TEST_F(LayouterTests, candidate_iteration_performance_fast_version) 
{
#ifdef EXPENSIVE_TESTS
  std::vector< std::string > names {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};

  std::vector<unsigned> atts;
  atts += 4, repeat(9, 4);

  #include "builder.h"

  Schema s(atts, 1000000, names);
  CandidateLayouter cl;

  for(size_t i=1; i <= names.size(); ++i)
  {
      auto intermediate_results = cl.iterateThroughLayoutSubsetsFast(i,
                                                                  data,
                                                                  0,
                                                                  i,
                                                                  names.size(),
                                                                  0);
      // std::cout << intermediate_results.size() << std::endl;
  }
#endif
}

} } // namespace hyrise::layouter

