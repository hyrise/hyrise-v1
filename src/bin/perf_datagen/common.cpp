// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * common.c
 *
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2002 Mark Wong & Open Source Development Lab, Inc.
 *
 * 16 may 2002
 * Based on TPC-C Standard Specification Revision 5.0.
 */

#include <pthread.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include "common.h"
#include "transaction_data.h"

char output_path[256] = "";
char a_string_char[A_STRING_CHAR_LEN];
const char* n_string_char = "0123456789";
const char* l_string_char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const char* c_last_syl[C_LAST_SYL_MAX] = {"BAR", "OUGHT", "ABLE",  "PRI",   "PRES",
                                          "ESE", "ANTI",  "CALLY", "ATION", "EING"};

const char transaction_short_name[TRANSACTION_MAX] = {'d', 'n', 'o', 'p', 's'};

const char* transaction_name[TRANSACTION_MAX] = {"delivery    ", "new-order   ", "order-status",
                                                 "payment     ", "stock-level "};

struct table_cardinality_t table_cardinality;

double difftimeval(struct timeval rt1, struct timeval rt0) {
  return (rt1.tv_sec - rt0.tv_sec) + (double)(rt1.tv_usec - rt0.tv_usec) / 1000000.00;
}

/* Clause 4.3.2.2.  */
void get_a_string(char* a_string, int x, int y) {
  int length;
  int i;

  length = x + get_random(y - x + 1) + 1;
  a_string[length - 1] = '\0';

  for (i = 0; i < length - 1; i++) {
    a_string[i] = a_string_char[get_random(A_STRING_CHAR_LEN - 1)];
  }

  return;
}

/* Clause 4.3.2.3 */
int get_c_last(char* c_last, int i) {
  char tmp[4];

  c_last[0] = '\0';

  if (i < 0 || i > 999) {
    return ERROR;
  }

  /* Ensure the number is padded with leading 0's if it's std::less than 100. */
  snprintf(tmp, 4, "%03d", i);

  strcat(c_last, c_last_syl[tmp[0] - '0']);
  strcat(c_last, c_last_syl[tmp[1] - '0']);
  strcat(c_last, c_last_syl[tmp[2] - '0']);
  return OK;
}

void get_l_string(char* a_string, int x, int y) {
  int length;
  int i;

  length = x + get_random(y - x + 1) + 1;
  a_string[length - 1] = '\0';

  for (i = 0; i < length - 1; i++) {
    a_string[i] = l_string_char[get_random(L_STRING_CHAR_LEN - 1)];
  }

  return;
}

/* Clause 4.3.2.2.  */
void get_n_string(char* n_string, int x, int y) {
  int length;
  int i;

  length = x + get_random(y - x + 1) + 1;
  n_string[length - 1] = '\0';

  for (i = 0; i < length - 1; i++) {
    n_string[i] = n_string_char[get_random(N_STRING_CHAR_LEN)];
  }

  return;
}

/* Clause 2.1.6 */
int get_nurand(int a, int x, int y) { return ((get_random(a + 1) | (x + get_random(y + 1))) % (y - x + 1)) + x; }

/* Return a number from 0 to max. */
double get_percentage() { return (double)rand() / (double)RAND_MAX; }

int get_random(int max) { return rand() % max; }

/*
 * Clause 5.2.5.4
 * Calculate and return a think time using a negative exponential function.
 * think_time = -ln(r) * m
 * return: think time, in milliseconds
 * r: random number, where 0 < r <= 1
 * mean_think_time = mean think time, in milliseconds
 */
int get_think_time(int mean_think_time) { return (-1.0 * log(get_percentage() + 0.000001) * mean_think_time); }

int init_common() {
  int i, j;

  srand(1);

  /* Initialize struct to have default table cardinalities. */
  table_cardinality.warehouses = 1;
  table_cardinality.districts = DISTRICT_CARDINALITY;
  table_cardinality.customers = CUSTOMER_CARDINALITY;
  table_cardinality.items = ITEM_CARDINALITY;
  table_cardinality.orders = ORDER_CARDINALITY;
  table_cardinality.new_orders = NEW_ORDER_CARDINALITY;

  /*
   * Initialize a-std::string character set to 128 ascii characters.
   * Clause 4.3.2.2.
   */
  j = 0;
  a_string_char[j++] = (char)33;
  for (i = 35; i <= 43; i++) {
    a_string_char[j++] = (char)i;
  }
  for (i = 45; i <= 126; i++) {
    if (i != 124) {
      a_string_char[j++] = (char)i;
    }
  }
  for (i = 220; i <= 255; i++) {
    a_string_char[j++] = (char)i;
  }

  return OK;
}
