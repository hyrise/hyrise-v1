// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * This file is released under the terms of the Artistic License.
 * Please see the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2002 Mark Wong & Open Source Development Labs, Inc.
 *
 * Based on TPC-C Standard Specification Revision 5.0.
 */

#include "common.h"
#include "datagen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define CUSTOMER_DATA "customer.tbl"
#define DISTRICT_DATA "district.tbl"
#define HISTORY_DATA "history.tbl"
#define ITEM_DATA "item.tbl"
#define NEW_ORDER_DATA "new_order.tbl"
#define ORDER_DATA "order.tbl"
#define ORDER_LINE_DATA "order_line.tbl"
#define STOCK_DATA "stock.tbl"
#define WAREHOUSE_DATA "warehouse.tbl"

#define MODE_SAPDB 0
#define MODE_PGSQL 1
#define MODE_MYSQL 2
#define MODE_HYRISE 3

void gen_customers();
void gen_districts();
void gen_history();
void gen_items();
void gen_new_order();
void gen_orders();
void gen_stock();
void gen_warehouses();

int warehouses = 0;
int customers = CUSTOMER_CARDINALITY;
int items = ITEM_CARDINALITY;
int orders = ORDER_CARDINALITY;
int new_orders = NEW_ORDER_CARDINALITY;

int mode_string = MODE_PGSQL;
char delimiter = ',';
char null_str[16] = "\"nullptr\"";

#define ERR_MSG(fn)                                                                  \
  {                                                                                  \
    (void) fflush(stderr);                                                           \
    (void) fprintf(stderr, __FILE__ ":%d:" #fn ": %s\n", __LINE__, strerror(errno)); \
  }
#define METAPRINTF(args) \
  if (fprintf args < 0)  \
  ERR_MSG(fn)

/* Oh my gosh, is there a better way to do this? */
#define FPRINTF(a, b, c)                                                                             \
  if (mode_string == MODE_SAPDB) {                                                                   \
    METAPRINTF((a, "\"" b "\"", c));                                                                 \
  } else if (mode_string == MODE_PGSQL || mode_string == MODE_MYSQL || mode_string == MODE_HYRISE) { \
    METAPRINTF((a, b, c));                                                                           \
  }
#define FPRINTF2(a, b)                                                                               \
  if (mode_string == MODE_SAPDB) {                                                                   \
    METAPRINTF((a, "\"" b "\""));                                                                    \
  } else if (mode_string == MODE_PGSQL || mode_string == MODE_MYSQL || mode_string == MODE_HYRISE) { \
    METAPRINTF((a, b));                                                                              \
  }

void escape_me(char* str) {
  /* Shouldn't need a buffer bigger than this. */
  char buffer[4096] = "";
  int i = 0;
  int j = 0;
  int k = 0;

  /* Don't need to do anything for SAP DB. */
  if (mode_string == MODE_PGSQL || mode_string == MODE_MYSQL) {
    strcpy(buffer, str);
    i = strlen(buffer);
    for (k = 0; k <= i; k++) {
      if (buffer[k] == '\\') {
        str[j++] = '\\';
      }
      str[j++] = buffer[k];
    }
  }
}

void print_timestamp(FILE* ofile, struct tm* date) {
  if (mode_string == MODE_SAPDB) {
    METAPRINTF((ofile,
                "\"%04d%02d%02d%02d%02d%02d000000\"",
                date->tm_year + 1900,
                date->tm_mon + 1,
                date->tm_mday,
                date->tm_hour,
                date->tm_min,
                date->tm_sec));
  } else if (mode_string == MODE_PGSQL || mode_string == MODE_MYSQL) {
    METAPRINTF((ofile,
                "%04d-%02d-%02d %02d:%02d:%02d",
                date->tm_year + 1900,
                date->tm_mon + 1,
                date->tm_mday,
                date->tm_hour,
                date->tm_min,
                date->tm_sec));
  } else if (mode_string == MODE_HYRISE) {
    METAPRINTF((ofile,
                "%04d-%02d-%02d-%02d-%02d-%02d",
                date->tm_year + 1900,
                date->tm_mon + 1,
                date->tm_mday,
                date->tm_hour,
                date->tm_min,
                date->tm_sec));

  } else {
    printf("unknown std::string mode: %d\n", mode_string);
    exit(1);
  }
}

// header function patched for hyrise - SB270911

void gen_table_header(FILE* output, std::string table_name) {
  std::vector<std::pair<std::string, std::string> > table_layout = layouts[table_name];
  std::stringstream fields;
  std::stringstream types;
  std::stringstream layout;
  size_t i = 0;

  for (std::vector<std::pair<std::string, std::string> >::iterator it = table_layout.begin(); it != table_layout.end();
       ++it, i++) {
    fields << (*it).first.c_str();
    types << (*it).second.c_str();
    layout << i << "_C";

    if (it != --table_layout.end()) {
      fields << delimiter;
      types << delimiter;
      layout << delimiter;
    } else {
      // this is the last field
      fields << "\n";
      types << "\n";
      layout << "\n";
    }
  }

  FPRINTF(output, "%s", fields.str().c_str());
  FPRINTF(output, "%s", types.str().c_str());
  FPRINTF(output, "%s", layout.str().c_str());

  // writing content delimiter
  METAPRINTF((output, "===\n"));
}

/* Clause 4.3.3.1 */
void gen_customers() {
  FILE* output;
  int i, j, k;
  char a_string[1024];
  struct tm* tm1;
  time_t t1;
  char filename[1024] = "\0";

  srand(0);
  printf("Generating customer table data...\n");

  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, CUSTOMER_DATA);
  output = fopen(filename, "w");
  if (output == nullptr) {
    printf("cannot open %s\n", CUSTOMER_DATA);
    return;
  }

  // patched for hyrise - SB270911
  if (mode_string == MODE_HYRISE) {
    gen_table_header(output, "customer");
  }

  for (i = 0; i < warehouses; i++) {
    for (j = 0; j < DISTRICT_CARDINALITY; j++) {
      for (k = 0; k < customers; k++) {
        /* c_id */
        FPRINTF(output, "%d", k + 1);
        METAPRINTF((output, "%c", delimiter));

        /* c_d_id */
        FPRINTF(output, "%d", j + 1);
        METAPRINTF((output, "%c", delimiter));

        /* c_w_id */
        FPRINTF(output, "%d", i + 1);
        METAPRINTF((output, "%c", delimiter));

        /* c_first */
        get_a_string(a_string, 8, 16);
        escape_me(a_string);
        FPRINTF(output, "%s", a_string);
        METAPRINTF((output, "%c", delimiter));

        /* c_middle */
        FPRINTF2(output, "OE");
        METAPRINTF((output, "%c", delimiter));

        /* c_last Clause 4.3.2.7 */
        if (k < 1000) {
          get_c_last(a_string, k);
        } else {
          get_c_last(a_string, get_nurand(255, 0, 999));
        }
        escape_me(a_string);
        FPRINTF(output, "%s", a_string);
        METAPRINTF((output, "%c", delimiter));

        /* c_street_1 */
        get_a_string(a_string, 10, 20);
        escape_me(a_string);
        FPRINTF(output, "%s", a_string);
        METAPRINTF((output, "%c", delimiter));

        /* c_street_2 */
        get_a_string(a_string, 10, 20);
        escape_me(a_string);
        FPRINTF(output, "%s", a_string);
        METAPRINTF((output, "%c", delimiter));

        /* c_city */
        get_a_string(a_string, 10, 20);
        escape_me(a_string);
        FPRINTF(output, "%s", a_string);
        METAPRINTF((output, "%c", delimiter));

        /* c_state */
        get_l_string(a_string, 2, 2);
        FPRINTF(output, "%s", a_string);
        METAPRINTF((output, "%c", delimiter));

        /* c_zip */
        get_n_string(a_string, 4, 4);
        FPRINTF(output, "%s11111", a_string);
        METAPRINTF((output, "%c", delimiter));

        /* c_phone */
        get_n_string(a_string, 16, 16);
        FPRINTF(output, "%s", a_string);
        METAPRINTF((output, "%c", delimiter));

        /* c_since */
        /*
         * Milliseconds are not calculated.  This
         * should also be the time when the data is
         * loaded, I think.
         */
        time(&t1);
        tm1 = localtime(&t1);
        print_timestamp(output, tm1);
        METAPRINTF((output, "%c", delimiter));

        /* c_credit */
        if (get_percentage() < .10) {
          FPRINTF2(output, "BC");
        } else {
          FPRINTF2(output, "GC");
        }
        METAPRINTF((output, "%c", delimiter));

        /* c_credit_lim */
        FPRINTF2(output, "50000.00");
        METAPRINTF((output, "%c", delimiter));

        /* c_discount */
        FPRINTF(output, "0.%04d", get_random(5000));
        METAPRINTF((output, "%c", delimiter));

        /* c_balance */
        FPRINTF2(output, "-10.00");
        METAPRINTF((output, "%c", delimiter));

        /* c_ytd_payment */
        FPRINTF2(output, "10.00");
        METAPRINTF((output, "%c", delimiter));

        /* c_payment_cnt */
        FPRINTF2(output, "1");
        METAPRINTF((output, "%c", delimiter));

        /* c_delivery_cnt */
        FPRINTF2(output, "0");
        METAPRINTF((output, "%c", delimiter));

        /* c_data */
        get_a_string(a_string, 300, 500);
        escape_me(a_string);
        FPRINTF(output, "%s", a_string);

        METAPRINTF((output, "\n"));
      }
    }
  }
  fclose(output);
  printf("Finished customer table data...\n");
  return;
}

/* Clause 4.3.3.1 */
void gen_districts() {
  FILE* output;
  int i, j;
  char a_string[48];
  char filename[1024] = "\0";

  srand(0);
  printf("Generating district table data...\n");

  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, DISTRICT_DATA);
  output = fopen(filename, "w");
  if (output == nullptr) {
    printf("cannot open %s\n", DISTRICT_DATA);
    return;
  }

  // patched for hyrise - SB270911
  if (mode_string == MODE_HYRISE) {
    gen_table_header(output, "district");
  }

  for (i = 0; i < warehouses; i++) {
    for (j = 0; j < DISTRICT_CARDINALITY; j++) {
      /* d_id */
      FPRINTF(output, "%d", j + 1);
      METAPRINTF((output, "%c", delimiter));

      /* d_w_id */
      FPRINTF(output, "%d", i + 1);
      METAPRINTF((output, "%c", delimiter));

      /* d_name */
      get_a_string(a_string, 6, 10);
      escape_me(a_string);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* d_street_1 */
      get_a_string(a_string, 10, 20);
      escape_me(a_string);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* d_street_2 */
      get_a_string(a_string, 10, 20);
      escape_me(a_string);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* d_city */
      get_a_string(a_string, 10, 20);
      escape_me(a_string);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* d_state */
      get_l_string(a_string, 2, 2);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* d_zip */
      get_n_string(a_string, 4, 4);
      FPRINTF(output, "%s11111", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* d_tax */
      FPRINTF(output, "0.%04d", get_random(2000));
      METAPRINTF((output, "%c", delimiter));

      /* d_ytd */
      FPRINTF2(output, "30000.00");
      METAPRINTF((output, "%c", delimiter));

      /* d_next_o_id */
      FPRINTF2(output, "3001");

      METAPRINTF((output, "\n"));
    }
  }
  fclose(output);
  printf("Finished district table data...\n");
  return;
}

/* Clause 4.3.3.1 */
void gen_history() {
  FILE* output;
  int i, j, k;
  char a_string[64];
  struct tm* tm1;
  time_t t1;
  char filename[1024] = "\0";

  srand(0);
  printf("Generating history table data...\n");

  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, HISTORY_DATA);
  output = fopen(filename, "w");
  if (output == nullptr) {
    printf("cannot open %s\n", HISTORY_DATA);
    return;
  }

  // patched for hyrise - SB270911
  if (mode_string == MODE_HYRISE) {
    gen_table_header(output, "history");
  }

  for (i = 0; i < warehouses; i++) {
    for (j = 0; j < DISTRICT_CARDINALITY; j++) {
      for (k = 0; k < customers; k++) {
        /* h_c_id */
        FPRINTF(output, "%d", k + 1);
        METAPRINTF((output, "%c", delimiter));

        /* h_c_d_id */
        FPRINTF(output, "%d", j + 1);
        METAPRINTF((output, "%c", delimiter));

        /* h_c_w_id */
        FPRINTF(output, "%d", i + 1);
        METAPRINTF((output, "%c", delimiter));

        /* h_d_id */
        FPRINTF(output, "%d", j + 1);
        METAPRINTF((output, "%c", delimiter));

        /* h_w_id */
        FPRINTF(output, "%d", i + 1);
        METAPRINTF((output, "%c", delimiter));

        /* h_date */
        /*
         * Milliseconds are not calculated.  This
         * should also be the time when the data is
         * loaded, I think.
         */
        time(&t1);
        tm1 = localtime(&t1);
        print_timestamp(output, tm1);
        METAPRINTF((output, "%c", delimiter));

        /* h_amount */
        FPRINTF2(output, "10.00");
        METAPRINTF((output, "%c", delimiter));

        /* h_data */
        get_a_string(a_string, 12, 24);
        escape_me(a_string);
        FPRINTF(output, "%s", a_string);

        METAPRINTF((output, "\n"));
      }
    }
  }
  fclose(output);
  printf("Finished history table data...\n");
  return;
}

/* Clause 4.3.3.1 */
void gen_items() {
  FILE* output;
  int i;
  char a_string[128];
  int j;
  char filename[1024] = "\0";

  srand(0);
  printf("Generating item table data...\n");

  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, ITEM_DATA);
  output = fopen(filename, "w");
  if (output == nullptr) {
    printf("cannot open %s\n", ITEM_DATA);
    return;
  }

  // patched for hyrise - SB270911
  if (mode_string == MODE_HYRISE) {
    gen_table_header(output, "item");
  }

  for (i = 0; i < items; i++) {
    /* i_id */
    FPRINTF(output, "%d", i + 1);
    METAPRINTF((output, "%c", delimiter));

    /* i_im_id */
    FPRINTF(output, "%d", get_random(9999) + 1);
    METAPRINTF((output, "%c", delimiter));

    /* i_name */
    get_a_string(a_string, 14, 24);
    escape_me(a_string);
    FPRINTF(output, "%s", a_string);
    METAPRINTF((output, "%c", delimiter));

    /* i_price */
    FPRINTF(output, "%0.2f", ((double)get_random(9900) + 100.0) / 100.0);
    METAPRINTF((output, "%c", delimiter));

    /* i_data */
    get_a_string(a_string, 26, 50);
    if (get_percentage() < .10) {
      j = get_random(strlen(a_string) - 8);
      strncpy(a_string + j, "ORIGINAL", 8);
    }
    escape_me(a_string);
    FPRINTF(output, "%s", a_string);

    METAPRINTF((output, "\n"));
  }
  fclose(output);
  printf("Finished item table data...\n");
  return;
}

/* Clause 4.3.3.1 */
void gen_new_orders() {
  FILE* output;
  int i, j, k;
  char filename[1024] = "\0";

  srand(0);
  printf("Generating new-order table data...\n");

  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, NEW_ORDER_DATA);
  output = fopen(filename, "w");
  if (output == nullptr) {
    printf("cannot open %s\n", NEW_ORDER_DATA);
    return;
  }

  // patched for hyrise - SB270911
  if (mode_string == MODE_HYRISE) {
    gen_table_header(output, "new_order");
  }

  for (i = 0; i < warehouses; i++) {
    for (j = 0; j < DISTRICT_CARDINALITY; j++) {
      for (k = orders - new_orders; k < orders; k++) {
        /* no_o_id */
        FPRINTF(output, "%d", k + 1);
        METAPRINTF((output, "%c", delimiter));

        /* no_d_id */
        FPRINTF(output, "%d", j + 1);
        METAPRINTF((output, "%c", delimiter));

        /* no_w_id */
        FPRINTF(output, "%d", i + 1);

        METAPRINTF((output, "\n"));
      }
    }
  }
  fclose(output);
  printf("Finished new-order table data...\n");
  return;
}

/* Clause 4.3.3.1 */
void gen_orders() {
  FILE* order, *order_line;
  int i, j, k, l;
  char a_string[64];
  struct tm* tm1;
  time_t t1;
  char filename[1024] = "\0";

  struct node_t {
    int value;
    struct node_t* next;
  };
  struct node_t* head;
  struct node_t* current;
  struct node_t* prev;
  struct node_t* new_node;
  int iter;

  int o_ol_cnt;

  srand(0);
  printf("Generating order and order-line table data...\n");

  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, ORDER_DATA);
  order = fopen(filename, "w");
  if (order == nullptr) {
    printf("cannot open %s\n", ORDER_DATA);
    return;
  }

  filename[0] = '\0';
  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, ORDER_LINE_DATA);
  order_line = fopen(filename, "w");
  if (order_line == nullptr) {
    printf("cannot open %s\n", ORDER_LINE_DATA);
    return;
  }

  // patched for hyrise - SB270911
  if (mode_string == MODE_HYRISE) {
    gen_table_header(order, "order");
    gen_table_header(order_line, "order_line");
  }

  for (i = 0; i < warehouses; i++) {
    for (j = 0; j < DISTRICT_CARDINALITY; j++) {
      /*
       * Create a random list of numbers from 1 to customers for o_c_id.
       */
      head = (struct node_t*)malloc(sizeof(struct node_t));
      head->value = 1;
      head->next = nullptr;
      for (k = 2; k <= customers; k++) {
        current = prev = head;

        /* Find a random place in the list to insert a number. */
        iter = get_random(k - 1);
        while (iter > 0) {
          prev = current;
          current = current->next;
          --iter;
        }

        /* Insert the number. */
        new_node = (struct node_t*)malloc(sizeof(struct node_t));
        if (current == prev) {
          /* Insert at the head of the list. */
          new_node->next = head;
          head = new_node;
        } else if (current == nullptr) {
          /* Insert at the tail of the list. */
          prev->next = new_node;
          new_node->next = nullptr;
        } else {
          /* Insert somewhere in the middle of the list. */
          prev->next = new_node;
          new_node->next = current;
        }
        new_node->value = k;
      }

      current = head;
      for (k = 0; k < orders; k++) {
        /* o_id */
        FPRINTF(order, "%d", k + 1);
        METAPRINTF((order, "%c", delimiter));

        /* o_d_id */
        FPRINTF(order, "%d", j + 1);
        METAPRINTF((order, "%c", delimiter));

        /* o_w_id */
        FPRINTF(order, "%d", i + 1);
        METAPRINTF((order, "%c", delimiter));

        /* o_c_id */
        FPRINTF(order, "%d", current->value);
        METAPRINTF((order, "%c", delimiter));
        current = current->next;

        /* o_entry_d */
        /*
         * Milliseconds are not calculated.  This
         * should also be the time when the data is
         * loaded, I think.
         */
        time(&t1);
        tm1 = localtime(&t1);
        print_timestamp(order, tm1);
        METAPRINTF((order, "%c", delimiter));

        /* o_carrier_id */
        if (k < 2101) {
          FPRINTF(order, "%d", get_random(9) + 1);
        } else {
          METAPRINTF((order, "%s", null_str));
        }
        METAPRINTF((order, "%c", delimiter));

        /* o_ol_cnt */
        o_ol_cnt = get_random(10) + 5;
        FPRINTF(order, "%d", o_ol_cnt);
        METAPRINTF((order, "%c", delimiter));

        /* o_all_local */
        FPRINTF2(order, "1");

        METAPRINTF((order, "\n"));

        /*
         * Generate data in the order-line table for
         * this order.
         */
        for (l = 0; l < o_ol_cnt; l++) {
          /* ol_o_id */
          FPRINTF(order_line, "%d", k + 1);
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_d_id */
          FPRINTF(order_line, "%d", j + 1);
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_w_id */
          FPRINTF(order_line, "%d", i + 1);
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_number */
          FPRINTF(order_line, "%d", l + 1);
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_i_id */
          FPRINTF(order_line, "%d", get_random(ITEM_CARDINALITY - 1) + 1);
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_supply_w_id */
          FPRINTF(order_line, "%d", i + 1);
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_delivery_d */
          if (k < 2101) {
            /*
             * Milliseconds are not
             * calculated.  This should
             * also be the time when the
             * data is loaded, I think.
             */
            time(&t1);
            tm1 = localtime(&t1);
            print_timestamp(order_line, tm1);
          } else {
            METAPRINTF((order_line, "%s", null_str));
          }
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_quantity */
          FPRINTF2(order_line, "5");
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_amount */
          if (k < 2101) {
            FPRINTF2(order_line, "0.00");
          } else {
            FPRINTF(order_line, "%f", (double)(get_random(999998) + 1) / 100.0);
          }
          METAPRINTF((order_line, "%c", delimiter));

          /* ol_dist_info */
          get_l_string(a_string, 24, 24);
          FPRINTF(order_line, "%s", a_string);

          METAPRINTF((order_line, "\n"));
        }
      }
      while (head != nullptr) {
        current = head;
        head = head->next;
        free(current);
      }
    }
  }
  fclose(order);
  fclose(order_line);
  printf("Finished order and order-line table data...\n");
  return;
}

/* Clause 4.3.3.1 */
void gen_stock() {
  FILE* output;
  int i, j, k;
  char a_string[128];
  char filename[1024] = "\0";

  srand(0);
  printf("Generating stock table data...\n");

  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, STOCK_DATA);
  output = fopen(filename, "w");
  if (output == nullptr) {
    printf("cannot open %s\n", STOCK_DATA);
    return;
  }

  // patched for hyrise - SB270911
  if (mode_string == MODE_HYRISE) {
    gen_table_header(output, "stock");
  }

  for (i = 0; i < warehouses; i++) {
    for (j = 0; j < items; j++) {
      /* s_i_id */
      FPRINTF(output, "%d", j + 1);
      METAPRINTF((output, "%c", delimiter));

      /* s_w_id */
      FPRINTF(output, "%d", i + 1);
      METAPRINTF((output, "%c", delimiter));

      /* s_quantity */
      FPRINTF(output, "%d", get_random(90) + 10);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_01 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_02 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_03 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_04 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_05 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_06 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_07 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_08 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_09 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_dist_10 */
      get_l_string(a_string, 24, 24);
      FPRINTF(output, "%s", a_string);
      METAPRINTF((output, "%c", delimiter));

      /* s_ytd */
      FPRINTF2(output, "0");
      METAPRINTF((output, "%c", delimiter));

      /* s_order_cnt */
      FPRINTF2(output, "0");
      METAPRINTF((output, "%c", delimiter));

      /* s_remote_cnt */
      FPRINTF2(output, "0");
      METAPRINTF((output, "%c", delimiter));

      /* s_data */
      get_a_string(a_string, 26, 50);
      if (get_percentage() < .10) {
        k = get_random(strlen(a_string) - 8);
        strncpy(a_string + k, "ORIGINAL", 8);
      }
      escape_me(a_string);
      FPRINTF(output, "%s", a_string);

      METAPRINTF((output, "\n"));
    }
  }
  fclose(output);
  printf("Finished stock table data...\n");
  return;
}

/* Clause 4.3.3.1 */
void gen_warehouses() {
  FILE* output;
  int i;
  char a_string[48];
  char filename[1024] = "\0";

  srand(0);
  printf("Generating warehouse table data...\n");

  if (strlen(output_path) > 0) {
    strcpy(filename, output_path);
    strcat(filename, "/");
  }
  strcat(filename, WAREHOUSE_DATA);

  output = fopen(filename, "w");
  if (output == nullptr) {
    printf("cannot open %s\n", WAREHOUSE_DATA);
    return;
  }

  // patched for hyrise - SB270911
  if (mode_string == MODE_HYRISE) {
    gen_table_header(output, "warehouse");
  }

  for (i = 0; i < warehouses; i++) {
    /* w_id */
    FPRINTF(output, "%d", i + 1);
    METAPRINTF((output, "%c", delimiter));

    /* w_name */
    get_a_string(a_string, 6, 10);
    escape_me(a_string);
    FPRINTF(output, "%s", a_string);
    METAPRINTF((output, "%c", delimiter));

    /* w_street_1 */
    get_a_string(a_string, 10, 20);
    escape_me(a_string);
    FPRINTF(output, "%s", a_string);
    METAPRINTF((output, "%c", delimiter));

    /* w_street_2 */
    get_a_string(a_string, 10, 20);
    escape_me(a_string);
    FPRINTF(output, "%s", a_string);
    METAPRINTF((output, "%c", delimiter));

    /* w_city */
    get_a_string(a_string, 10, 20);
    escape_me(a_string);
    FPRINTF(output, "%s", a_string);
    METAPRINTF((output, "%c", delimiter));

    /* w_state */
    get_l_string(a_string, 2, 2);
    FPRINTF(output, "%s", a_string);
    METAPRINTF((output, "%c", delimiter));

    /* w_zip */
    get_n_string(a_string, 4, 4);
    FPRINTF(output, "%s11111", a_string);
    METAPRINTF((output, "%c", delimiter));

    /* w_tax */
    FPRINTF(output, "0.%04d", get_random(2000));
    METAPRINTF((output, "%c", delimiter));

    /* w_ytd */
    FPRINTF2(output, "300000.00");

    METAPRINTF((output, "\n"));
  }
  fclose(output);
  printf("Finished warehouse table data...\n");
  return;
}

int main(int argc, char* argv[]) {
  struct stat st;

  /* For getoptlong(). */
  int c;

  init_common();

  if (argc < 2) {
    printf("Usage: %s -w # [-c #] [-i #] [-o #] [-s #] [-n #] [-d <str>]\n", argv[0]);
    printf("\n");
    printf("-w #\n");
    printf("\twarehouse cardinality\n");
    printf("-c #\n");
    printf("\tcustomer cardinality, default %d\n", CUSTOMER_CARDINALITY);
    printf("-i #\n");
    printf("\titem cardinality, default %d\n", ITEM_CARDINALITY);
    printf("-o #\n");
    printf("\torder cardinality, default %d\n", ORDER_CARDINALITY);
    printf("-n #\n");
    printf("\tnew-order cardinality, default %d\n", NEW_ORDER_CARDINALITY);
    printf("-d <path>\n");
    printf("\toutput path of data files\n");
    printf("--sapdb\n");
    printf("\tformat data for SAP DB\n");
    printf("--pgsql\n");
    printf("\tformat data for PostgreSQL\n");
    printf("--mysql\n");
    printf("\tformat data for MySQL\n");
    printf("--hyrise\n");  // patched for hyrise
    printf("\tformat data for HYRISE\n");  // SB270911
    return 1;
  }

  /* Parse command line arguments. */
  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
        {"pgsql", no_argument, &mode_string, MODE_PGSQL},
        {"sapdb", no_argument, &mode_string, MODE_SAPDB},
        {"mysql", no_argument, &mode_string, MODE_MYSQL},
        {"hyrise", no_argument, &mode_string, MODE_HYRISE},  // patched for hyrise, SB270911
        {0, 0, 0, 0}};

    c = getopt_long(argc, argv, "c:d:i:n:o:w:", long_options, &option_index);
    if (c == -1) {
      break;
    }

    switch (c) {
      case 0:
        break;
      case 'c':
        customers = atoi(optarg);
        break;
      case 'd':
        strcpy(output_path, optarg);
        break;
      case 'i':
        items = atoi(optarg);
        break;
      case 'n':
        new_orders = atoi(optarg);
        break;
      case 'o':
        orders = atoi(optarg);
        break;
      case 'w':
        warehouses = atoi(optarg);
        break;
      default:
        printf("?? getopt returned character code 0%o ??\n", c);
        return 2;
    }
  }

  if (warehouses == 0) {
    printf("-w must be used\n");
    return 3;
  }

  if (strlen(output_path) > 0 && ((stat(output_path, &st) < 0) || (st.st_mode & S_IFMT) != S_IFDIR)) {
    printf("Output directory of data files '%s' not exists\n", output_path);
    return 3;
  }

  /* Set the correct delimiter. */
  if (mode_string == MODE_SAPDB) {
    delimiter = ',';
    strcpy(null_str, "\"nullptr\"");
  } else if (mode_string == MODE_PGSQL || mode_string == MODE_MYSQL) {
    delimiter = '\t';
    strcpy(null_str, "");
  } else if (mode_string == MODE_HYRISE) {  // patched for hyrise
    delimiter = '|';  //
    strcpy(null_str, "");  //
    loadHyriseLayouts();  // SB270911
  }

  printf("warehouses = %d\n", warehouses);
  printf("districts = %d\n", DISTRICT_CARDINALITY);
  printf("customers = %d\n", customers);
  printf("items = %d\n", items);
  printf("orders = %d\n", orders);
  printf("stock = %d\n", items);
  printf("new_orders = %d\n", new_orders);
  printf("\n");

  if (strlen(output_path) > 0) {
    printf("Output directory of data files: %s\n", output_path);
  } else {
    printf("Output directory of data files: current directory\n");
  }
  printf("\n");

  printf("Generating data files for %d warehouse(s)...\n", warehouses);

  gen_items();
  gen_warehouses();
  gen_stock();
  gen_districts();
  gen_customers();
  gen_history();
  gen_orders();
  gen_new_orders();

  return 0;
}
