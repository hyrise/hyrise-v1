// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/storage_types_helper.h"

#include <stdexcept>
#include <string>
#include <algorithm>

namespace hyrise {

std::string data_type_to_string(DataType d) {
  switch (d) {
  case IntegerType:
    return types::integer_name_main;
  case IntegerTypeDelta:
    return types::integer_name;
  case IntegerTypeDeltaConcurrent:
    return types::integer_name_conc;
  case FloatType:
    return types::float_name_main;
  case FloatTypeDelta:
    return types::float_name;
  case FloatTypeDeltaConcurrent:
    return types::float_name_conc;
  case FloatNoDictType:
    return types::no_dict_float_name;
  case StringType:
    return types::string_name_main;
  case StringTypeDelta:
    return types::string_name;
  case StringTypeDeltaConcurrent:
    return types::string_name_conc;
  case IntegerNoDictType:
    return types::no_dict_integer_name;
  default:
      throw std::runtime_error("Type does not match");
  }
}

pos_list_t *pos_list_intersection(pos_list_t *p1, pos_list_t *p2, const bool deleteInput) {
  size_t max_size = p1->size() < p2->size() ? p1->size() : p2->size();

  pos_list_t *result = new pos_list_t(max_size);

  pos_list_t::iterator it = std::set_intersection(p1->begin(), p1->end(), p2->begin(), p2->end(), result->begin());

  result->erase(it, result->end());

  if (deleteInput) {
    delete p1;
    delete p2;
  }

  return result;
}

} // namespace hyrise

