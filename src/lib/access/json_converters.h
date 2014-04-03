// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_JSON_CONVERTERS_H_
#define SRC_LIB_ACCESS_JSON_CONVERTERS_H_

#include <helper/types.h>
#include <storage/AbstractTable.h>
#include <json.h>

struct json_converter {
  template <typename T>
  static T convert(Json::Value v);
};

struct set_json_value_functor {

  typedef void value_type;

  hyrise::storage::atable_ptr_t tab;
  size_t col;
  size_t row;
  Json::Value val;


  inline set_json_value_functor(hyrise::storage::atable_ptr_t t) : tab(t) {}

  inline void set(size_t c, size_t r, Json::Value v) {
    col = c;
    row = r;
    val = v;
  }

  template <typename T>
  value_type operator()() {
    tab->setValue(col, row, json_converter::convert<T>(val));
  }
};

#endif  // SRC_LIB_ACCESS_JSON_CONVERTERS_H_
