// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2002 Mark Wong & Open Source Development Labs, Inc.
 *
 * 16 may 2002
 */

#ifndef SRC_BIN_PERF_DATAGEN_COMMON_H_
#define SRC_BIN_PERF_DATAGEN_COMMON_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>
#include <map>
#include <utility>
#include <string>

#if defined(ODBC) || defined(LIBMYSQL)
#define DB_USER "dbt"
#define DB_PASS ""
#endif /* ODBC || LIBMYSQL */

#if defined(LIBPQ) || defined(LIBMYSQL)
#define DB_NAME "dbt2"
#endif /* LIBPQ || LIBMYSQL */

#define DELIVERY 0
#define NEW_ORDER 1
#define ORDER_STATUS 2
#define PAYMENT 3
#define STOCK_LEVEL 4
#define TRANSACTION_MAX 5
#define INTEGRITY 10

#define TABLE_WAREHOUSE 0
#define TABLE_DISTRICT 1
#define TABLE_CUSTOMER 2
#define TABLE_ITEM 3
#define TABLE_ORDER 4
#define TABLE_STOCK 5
#define TABLE_NEW_ORDER 6

#define ERROR 0
#define OK 1
#define EXIT_CODE 2
#define ERROR_SOCKET_CLOSED 3
#define STATUS_ROLLBACK 4
#define ERROR_RECEIVE_TIMEOUT 5

#define A_STRING_CHAR_LEN 128
#define L_STRING_CHAR_LEN 52
#define N_STRING_CHAR_LEN 10
#define TIMESTAMP_LEN 28

#define CUSTOMER_CARDINALITY 3000
#define DISTRICT_CARDINALITY 10
#define ITEM_CARDINALITY 100000
#define ORDER_CARDINALITY 3000
#define STOCK_CARDINALITY 100000
#define NEW_ORDER_CARDINALITY 900

#define D_CITY_LEN 20
#define D_NAME_LEN 10
#define D_STATE_LEN 2
#define D_STREET_1_LEN 20
#define D_STREET_2_LEN 20
#define D_ZIP_LEN 9

#define C_CREDIT_LEN 2
#define C_DATA_LEN 500
#define C_FIRST_LEN 16
#define C_LAST_LEN 16
#define C_MIDDLE_LEN 2
#define C_STREET_1_LEN 20
#define C_STREET_2_LEN 20
#define C_CITY_LEN 20
#define C_PHONE_LEN 16
#define C_SINCE_LEN TIMESTAMP_LEN
#define C_STATE_LEN 2
#define C_ZIP_LEN 9

#define I_NAME_LEN 24

#define O_ENTRY_D_LEN TIMESTAMP_LEN

#define OL_DELIVERY_D_LEN TIMESTAMP_LEN

#define W_CITY_LEN 20
#define W_NAME_LEN 10
#define W_STATE_LEN 2
#define W_STREET_1_LEN 20
#define W_STREET_2_LEN 20
#define W_ZIP_LEN 9

#define C_ID_UNKNOWN 0
#define C_LAST_SYL_MAX 10
#define D_ID_MAX 10
#define O_OL_CNT_MAX 15
#define O_CARRIER_ID_MAX 10

#define CLIENT_PORT 30000

#define CLIENT_PID_FILENAME "dbt2_client.pid"
#define DRIVER_PID_FILENAME "dbt2_driver.pid"


struct table_cardinality_t {
  int warehouses;
  int districts;
  int customers;
  int items;
  int orders;
  int new_orders;
};

/* Prototypes */
double difftimeval(struct timeval rt1, struct timeval rt0);
int edump(int type, void* data);
void get_a_string(char* a_string, int x, int y);
int get_c_last(char* c_last, int i);
void get_l_string(char* l_string, int x, int y);
void get_n_string(char* n_string, int x, int y);
int get_nurand(int a, int x, int y);
double get_percentage();
int get_random(int max);
int get_think_time(int mean_think_time);
int init_common();
int create_pid_file();

extern char output_path[256];
extern const char* c_last_syl[C_LAST_SYL_MAX];
extern struct table_cardinality_t table_cardinality;
extern const char transaction_short_name[TRANSACTION_MAX];
extern const char* transaction_name[TRANSACTION_MAX];

#endif  // SRC_BIN_PERF_DATAGEN_COMMON_H_
