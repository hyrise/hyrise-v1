// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <gtest/gtest-bench.h>
#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <access.h>
#include <storage.h>
#include <io.h>

namespace hyrise {
namespace access {

//See TPC-C reference Page 108 Chapter A.3 (New-Order Transaction)
//Skipping projection since this is benchmarked in ProjectionScan.cpp
class SimpleTableScanBase : public ::testing::Benchmark {

 protected:

  io::StorageManager *sm;
  SimpleTableScan *ts;
  storage::c_atable_ptr_t order_line;
  storage::c_atable_ptr_t customer;

 public:
  void BenchmarkSetUp() {
    ts = new SimpleTableScan();
    ts->setEvent("NO_PAPI");

    sm = io::StorageManager::getInstance();
    order_line = sm->getTable("order_line");
    customer = sm->getTable("customer");
  }

  void BenchmarkTearDown() {
  }

  SimpleTableScanBase() {
    SetNumIterations(10);
    SetWarmUp(2);
  }
};



/*
 * We do no projection, thus some colums are getting fixed values

 SELECT c_balance,
 c_first,
 c_middle,
 c_last
 FROM
 customer
 WHERE
 c_id = 2564
 AND
 c_d_id = 9
 AND
 c_w_id = 1
*/

CompoundExpression *customer_selection(hyrise::storage::c_atable_ptr_t &customer) {
  auto
      expr_c_id = new EqualsExpression<hyrise_int_t>(customer, 0, 2564l),
      expr_c_d_id = new EqualsExpression<hyrise_int_t>(customer, 1, 9l),
      expr_c_w_id = new EqualsExpression<hyrise_int_t>(customer, 2, 1l);

  auto
      first_and = new CompoundExpression(AND),
      scnd_and = new CompoundExpression(AND);

  first_and->add(expr_c_id);
  first_and->add(expr_c_d_id);
  scnd_and->add(first_and);
  scnd_and->add(expr_c_w_id);
  return scnd_and;
}

BENCHMARK_F(SimpleTableScanBase, single_select) {
  ts->addInput(customer);
  auto expr_c_id = new EqualsExpression<hyrise_int_t>(customer, 0, 1l);
  ts->setPredicate(expr_c_id);
  ts->setProducesPositions(true);

  auto result = ts->execute()->getResultTable();
}

BENCHMARK_F(SimpleTableScanBase, simple_test_scan) {
  ts->addInput(customer);
  ts->setPredicate(customer_selection(customer));
  ts->setProducesPositions(true);

  auto result = ts->execute()->getResultTable();
}


BENCHMARK_F(SimpleTableScanBase, table_scan_order_status_cust_tpcc_mat) {
  ts->addInput(customer);
  ts->setPredicate(customer_selection(customer));
  ts->setProducesPositions(true);

  auto result = ts->execute()->getResultTable();

  MaterializingScan ms(false);
  ms.setEvent("NO_PAPI");
  ms.addInput(result);

  auto result_mat = ms.execute()->getResultTable();
}

BENCHMARK_F(SimpleTableScanBase, table_scan_order_status_cust_tpcc_mat_memcpy) {
  ts->addInput(customer);
  ts->setPredicate(customer_selection(customer));
  ts->setProducesPositions(true);

  auto result = ts->execute()->getResultTable();

  MaterializingScan ms(true);
  ms.setEvent("NO_PAPI");
  ms.addInput(result);

  auto result_mat = ms.execute()->getResultTable();
}

BENCHMARK_F(SimpleTableScanBase, table_scan_order_status_line_tpcc) {
  /*
    SELECT o_i_id,
    ol_supply_w_id,
    ol_quantity
    FROM
    order_line
    WHERE
    ol_o_id = 1
    AND
    ol_d_id = 6 */

  ts->addInput(order_line);

  EqualsExpression<hyrise_int_t> *expr_ol_o_id = new EqualsExpression<hyrise_int_t>(order_line, 0, 2758);
  EqualsExpression<hyrise_int_t> *expr_ol_d_id = new EqualsExpression<hyrise_int_t>(order_line, 1, 10);

  CompoundExpression *expr_and = new CompoundExpression(AND);

  expr_and->add(expr_ol_o_id);
  expr_and->add(expr_ol_d_id);

  ts->setPredicate(expr_and);
  ts->setProducesPositions(true);

  auto result = ts->execute()->getResultTable();
}

BENCHMARK_F(SimpleTableScanBase, table_scan_order_status_line_tpcc_mat) {
  /*
    SELECT o_i_id,
    ol_supply_w_id,
    ol_quantity
    FROM
    order_line
    WHERE
    ol_o_id = 1
    AND
    ol_d_id = 6 */

  ts->addInput(order_line);

  EqualsExpression<hyrise_int_t> *expr_ol_o_id = new EqualsExpression<hyrise_int_t>(order_line, 0, 2758);
  EqualsExpression<hyrise_int_t> *expr_ol_d_id = new EqualsExpression<hyrise_int_t>(order_line, 1, 10);

  CompoundExpression *expr_and = new CompoundExpression(AND);

  expr_and->add(expr_ol_o_id);
  expr_and->add(expr_ol_d_id);

  ts->setPredicate(expr_and);
  ts->setProducesPositions(true);

  auto result = ts->execute()->getResultTable();
}

BENCHMARK_F(SimpleTableScanBase, table_scan_order_status_line_tpcc_mat_memcpy) {
  /*
    SELECT o_i_id,
    ol_supply_w_id,
    ol_quantity
    FROM
    order_line
    WHERE
    ol_o_id = 1
    AND
    ol_d_id = 6 */

  ts->addInput(order_line);

  EqualsExpression<hyrise_int_t> *expr_ol_o_id = new EqualsExpression<hyrise_int_t>(order_line, 0, 2758);
  EqualsExpression<hyrise_int_t> *expr_ol_d_id = new EqualsExpression<hyrise_int_t>(order_line, 1, 10);

  CompoundExpression *expr_and = new CompoundExpression(AND);

  expr_and->add(expr_ol_o_id);
  expr_and->add(expr_ol_d_id);

  ts->setPredicate(expr_and);
  ts->setProducesPositions(true);

  auto result = ts->execute()->getResultTable();
}

}
}
