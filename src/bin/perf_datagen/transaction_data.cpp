// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2002 Mark Wong & Open Source Development Lab, Inc.
 *
 * 24 june 2002
 */

#include "transaction_data.h"
#include <pthread.h>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
union data_pointer_t {
  struct delivery_t* de;
  struct new_order_t* no;
  struct order_status_t* os;
  struct payment_t* pa;
  struct stock_level_t* sl;
};

int dump(FILE* fp, int type, void* data) {
  int i;
  union data_pointer_t ptr;

  switch (type) {
    case DELIVERY:
      ptr.de = (struct delivery_t*)data;
      pthread_mutex_lock(&mut);
      fprintf(fp, "w_id = %d\n", ptr.de->w_id);
      fprintf(fp, "o_carrier_id = %d\n", ptr.de->o_carrier_id);
      pthread_mutex_unlock(&mut);
      break;
    case NEW_ORDER:
      ptr.no = (struct new_order_t*)data;
      pthread_mutex_lock(&mut);
      fprintf(fp, "w_id = %d\n", ptr.no->w_id);
      fprintf(fp, "w_tax = %0.4f\n", ptr.no->w_tax);
      fprintf(fp, "d_id = %d\n", ptr.no->d_id);
      fprintf(fp, "d_tax = %0.4f\n", ptr.no->d_tax);
      fprintf(fp, "c_id = %d\n", ptr.no->c_id);
      fprintf(fp, "c_last = %s\n", ptr.no->c_last);
      fprintf(fp, "c_credit = %s\n", ptr.no->c_credit);
      fprintf(fp, "c_discount = %0.4f\n", ptr.no->c_discount);
      fprintf(fp, "o_all_local = %d\n", ptr.no->o_all_local);
      fprintf(fp, "o_ol_cnt = %d\n", ptr.no->o_ol_cnt);
      fprintf(fp,
              "%-2s %-7s %-24s %-9s %-14s %-11s %-10s %-10s\n",
              "##",
              "ol_i_id",
              "i_name",
              "i_price",
              "ol_supply_w_id",
              "ol_quantity",
              "s_quantity",
              "ol_ammount");
      fprintf(fp,
              "%-2s %-7s %-24s %-9s %-14s %-11s %-10s %-10s\n",
              "--",
              "-------",
              "------------------------",
              "---------",
              "--------------",
              "-----------",
              "----------",
              "----------");
      for (i = 0; i < ptr.no->o_ol_cnt; i++) {
        fprintf(fp,
                "%2d %7d %24s %9.2f %14d %11d %10d %10.2f\n",
                i + 1,
                ptr.no->order_line[i].ol_i_id,
                ptr.no->order_line[i].i_name,
                ptr.no->order_line[i].i_price,
                ptr.no->order_line[i].ol_supply_w_id,
                ptr.no->order_line[i].ol_quantity,
                ptr.no->order_line[i].s_quantity,
                ptr.no->order_line[i].ol_amount);
      }
      fprintf(fp, "o_id = %d\n", ptr.no->o_id);
      fprintf(fp, "total_amount = %0.2f\n", ptr.no->total_amount);
      pthread_mutex_unlock(&mut);
      pthread_mutex_unlock(&mut);
      break;
    case ORDER_STATUS:
      ptr.os = (struct order_status_t*)data;
      pthread_mutex_lock(&mut);
      fprintf(fp, "c_id = %d\n", ptr.os->c_id);
      fprintf(fp, "c_w_id = %d\n", ptr.os->c_w_id);
      fprintf(fp, "c_d_id = %d\n", ptr.os->c_d_id);
      fprintf(fp, "c_first = %s\n", ptr.os->c_first);
      fprintf(fp, "c_middle = %s\n", ptr.os->c_middle);
      fprintf(fp, "c_last = %s\n", ptr.os->c_last);
      fprintf(fp, "c_balance = %0.2f\n", ptr.os->c_balance);
      fprintf(fp, "o_id = %d\n", ptr.os->o_id);
      fprintf(fp, "o_carrier_id = %d\n", ptr.os->o_carrier_id);
      fprintf(fp, "o_entry_d = %s\n", ptr.os->o_entry_d);
      fprintf(fp, "o_ol_cnt = %d\n", ptr.os->o_ol_cnt);
      fprintf(fp, "##  ol_i_id  ol_supply_w_id  ol_quantity  ol_amount  ol_delivery_d\n");
      for (i = 0; i < ptr.os->o_ol_cnt; i++) {
        fprintf(fp,
                "%2d  %7d  %14d  %11d  %9.2f  %s\n",
                i,
                ptr.os->order_line[i].ol_i_id,
                ptr.os->order_line[i].ol_supply_w_id,
                ptr.os->order_line[i].ol_quantity,
                ptr.os->order_line[i].ol_amount,
                ptr.os->order_line[i].ol_delivery_d);
      }
      pthread_mutex_unlock(&mut);
      break;
    case PAYMENT:
      ptr.pa = (struct payment_t*)data;
      pthread_mutex_lock(&mut);
      fprintf(fp, "w_id = %d\n", ptr.pa->w_id);
      fprintf(fp, "w_name = %s\n", ptr.pa->w_name);
      fprintf(fp, "w_street_1 = %s\n", ptr.pa->w_street_1);
      fprintf(fp, "w_street_2 = %s\n", ptr.pa->w_street_2);
      fprintf(fp, "w_city = %s\n", ptr.pa->w_city);
      fprintf(fp, "w_state = %s\n", ptr.pa->w_state);
      fprintf(fp, "w_zip = %s\n", ptr.pa->w_zip);
      fprintf(fp, "d_id = %d\n", ptr.pa->d_id);
      fprintf(fp, "d_name = %s\n", ptr.pa->d_name);
      fprintf(fp, "d_street_1 = %s\n", ptr.pa->d_street_1);
      fprintf(fp, "d_street_2 = %s\n", ptr.pa->d_street_2);
      fprintf(fp, "d_city = %s\n", ptr.pa->d_city);
      fprintf(fp, "d_state = %s\n", ptr.pa->d_state);
      fprintf(fp, "d_zip = %s\n", ptr.pa->d_zip);
      fprintf(fp, "c_id = %d\n", ptr.pa->c_id);
      fprintf(fp, "c_last = %s\n", ptr.pa->c_last);
      fprintf(fp, "c_w_id = %d\n", ptr.pa->c_w_id);
      fprintf(fp, "c_d_id = %d\n", ptr.pa->c_d_id);
      fprintf(fp, "c_first = %s\n", ptr.pa->c_first);
      fprintf(fp, "c_middle = %s\n", ptr.pa->c_middle);
      fprintf(fp, "c_street_1 = %s\n", ptr.pa->c_street_1);
      fprintf(fp, "c_street_2 = %s\n", ptr.pa->c_street_2);
      fprintf(fp, "c_city = %s\n", ptr.pa->c_city);
      fprintf(fp, "c_state = %s\n", ptr.pa->c_state);
      fprintf(fp, "c_zip = %s\n", ptr.pa->c_zip);
      fprintf(fp, "c_phone = %s\n", ptr.pa->c_phone);
      fprintf(fp, "c_since = %s\n", ptr.pa->c_since);
      fprintf(fp, "c_credit = %s\n", ptr.pa->c_credit);
      fprintf(fp, "c_credit_lim = %0.2f\n", ptr.pa->c_credit_lim);
      fprintf(fp, "c_discount = %0.4f\n", ptr.pa->c_discount);
      fprintf(fp, "c_balance = %0.2f\n", ptr.pa->c_balance);
      fprintf(fp, "c_data = %s\n", ptr.pa->c_data);
      fprintf(fp, "h_amount = %0.2f\n", ptr.pa->h_amount);
      pthread_mutex_unlock(&mut);
      break;
    case STOCK_LEVEL:
      ptr.sl = (struct stock_level_t*)data;
      pthread_mutex_lock(&mut);
      fprintf(fp, "w_id = %d\n", ptr.sl->w_id);
      fprintf(fp, "d_id = %d\n", ptr.sl->d_id);
      fprintf(fp, "threshold = %d\n", ptr.sl->threshold);
      fprintf(fp, "low_stock = %d\n", ptr.sl->low_stock);
      pthread_mutex_unlock(&mut);
      break;
    default:
      return ERROR;
  }
  return OK;
}
