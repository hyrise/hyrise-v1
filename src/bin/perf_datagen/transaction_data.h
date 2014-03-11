// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * transaction_data.h
 *
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2002 Mark Wong & Open Source Development Lab, Inc.
 *
 * 11 june 2002
 * Based on TPC-C Standard Specification Revision 5.0.
 */

#ifndef SRC_BIN_PERF_DATAGEN_TRANSACTION_DATA_H_
#define SRC_BIN_PERF_DATAGEN_TRANSACTION_DATA_H_

#include "common.h"

struct delivery_t {
  /* Input data. */
  int w_id;
  int o_carrier_id;
};

struct no_order_line_t {
  int ol_i_id;
  int ol_supply_w_id;
  int ol_quantity;
  double ol_amount;
  int s_quantity;
  char brand_generic;
  char i_name[I_NAME_LEN + 1];
  double i_price;
};

struct new_order_t {
  /* Input data. */
  int w_id;
  int d_id;
  int c_id;
  int o_ol_cnt;
  int o_all_local;
  struct no_order_line_t order_line[O_OL_CNT_MAX];

  /* Output data. */
  int o_id;
  double total_amount;
  double w_tax;
  double d_tax;
  char c_last[C_LAST_LEN + 1];
  char c_credit[C_CREDIT_LEN + 1];
  double c_discount;
  int rollback;
};

struct payment_t {
  /* Input data. */
  int c_d_id;
  int c_w_id;
  int d_id;
  double h_amount;
  int w_id;

  /* Input and output data. */
  int c_id;
  char c_last[C_LAST_LEN + 1];

  /* Output data. */
  char d_name[D_NAME_LEN + 1];
  char d_street_1[D_STREET_1_LEN + 1];
  char d_street_2[D_STREET_2_LEN + 1];
  char d_city[D_CITY_LEN + 1];
  char d_state[D_STATE_LEN + 1];
  char d_zip[D_ZIP_LEN + 1];
  double c_balance;
  double c_credit_lim;
  double c_discount;
  char c_data[C_DATA_LEN + 1];
  char c_first[C_FIRST_LEN + 1];
  char c_middle[C_MIDDLE_LEN + 1];
  char c_street_1[C_STREET_1_LEN + 1];
  char c_street_2[C_STREET_2_LEN + 1];
  char c_city[C_CITY_LEN + 1];
  char c_state[C_STATE_LEN + 1];
  char c_zip[C_ZIP_LEN + 1];
  char c_phone[C_PHONE_LEN + 1];
  char c_since[C_SINCE_LEN + 1];
  char c_credit[C_CREDIT_LEN + 1];
  char w_name[W_NAME_LEN + 1];
  char w_street_1[W_STREET_1_LEN + 1];
  char w_street_2[W_STREET_2_LEN + 1];
  char w_city[W_CITY_LEN + 1];
  char w_state[W_STATE_LEN + 1];
  char w_zip[W_ZIP_LEN + 1];
};

struct os_order_line_t {
  int ol_i_id;
  int ol_supply_w_id;
  int ol_quantity;
  double ol_amount;
  char ol_delivery_d[OL_DELIVERY_D_LEN + 1];
};

struct order_status_t {
  /* Input data. */
  int c_d_id;
  int c_w_id;

  /* Input and output data. */
  int c_id;
  char c_last[C_LAST_LEN + 1];

  /* Output data. */
  double c_balance;
  char c_first[C_FIRST_LEN + 1];
  char c_middle[C_MIDDLE_LEN + 1];
  int o_id;
  int o_carrier_id;
  char o_entry_d[O_ENTRY_D_LEN + 1];
  int o_ol_cnt;
  struct os_order_line_t order_line[O_OL_CNT_MAX];
};

struct stock_level_t {
  /* Input data. */
  int w_id;
  int d_id;
  int threshold;

  /* Output data. */
  int low_stock;
};

struct integrity_t {
  /* Input data. */
  int w_id;
};

union transaction_data_t {
  struct delivery_t delivery;
  struct new_order_t new_order;
  struct order_status_t order_status;
  struct payment_t payment;
  struct stock_level_t stock_level;
  struct integrity_t integrity;
};

int dump(FILE* fp, int type, void* data);

#endif  // SRC_BIN_PERF_DATAGEN_TRANSACTION_DATA_H_
