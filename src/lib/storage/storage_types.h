// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_STORAGE_TYPES_H_
#define SRC_LIB_STORAGE_STORAGE_TYPES_H_

#include <vector>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <ostream>


#define STORAGE_XSTR(x) STORAGE_STR(x)
#define STORAGE_STR(x) #x
#define STORAGE_DEBUG_WHERE_WHAT(file, line)  std::string(#file ":" #line)
#define STORAGE_NOT_IMPLEMENTED(class, method) throw std::runtime_error(#class "(" STORAGE_XSTR(__FILE__) ":" STORAGE_XSTR(__LINE__) ") does not "  #method); 

typedef enum {
  IntegerType,
  FloatType,
  StringType
} DataType;


#define STR_TYPE_INTEGER "INTEGER"
#define STR_TYPE_FLOAT "FLOAT"
#define STR_TYPE_STRING "STRING"

namespace hyrise {
namespace types {
typedef std::string type_t;
const std::string integer_t = STR_TYPE_INTEGER;
const std::string float_t = STR_TYPE_FLOAT;
const std::string string_t = STR_TYPE_STRING;
}
}

typedef int64_t hyrise_int_t;
typedef float hyrise_float_t;
typedef std::string hyrise_string_t;

typedef unsigned int value_id_t;
typedef unsigned char table_id_t;

typedef size_t pos_t;
typedef size_t field_t;

typedef std::string field_name_t;
typedef std::vector<field_name_t> field_name_list_t;

typedef std::vector<pos_t> pos_list_t;
typedef std::vector<field_t> field_list_t;


/*
  This is the ValueID class used to store important information
*/
class ValueId {
 public:
  value_id_t valueId;
  table_id_t table;

  ValueId() { }
  ValueId(value_id_t _valueId, table_id_t _table) : valueId(_valueId), table(_table) { }


  bool operator==(ValueId &o) {
    return valueId == o.valueId && table == o.table;
  }

  bool operator!=(ValueId &o) {
    return valueId != o.valueId && table == o.table;
  }
  bool operator<(const ValueId &o) const {
    if (table!=o.table)
      throw std::runtime_error("comparing value ids of different tables");
    return valueId<o.valueId;
  }

  bool operator>(const ValueId &o) const {
    if (table!=o.table)
      throw std::runtime_error("comparing value ids of different tables");
    return valueId>o.valueId;
  }

  friend inline std::ostream& operator<<(std::ostream& os, const ValueId& v) {
    os << "<ValueID v:" << v.valueId << " t:" << (int) v.table << ">";
    return os;
  }
};

typedef std::vector<ValueId> ValueIdList;

class ColumnMetadata;
typedef std::vector<const ColumnMetadata *> metadata_list;
typedef std::vector<metadata_list *> compound_metadata_list;
typedef std::vector<ColumnMetadata> metadata_vec_t;

#endif  // SRC_LIB_STORAGE_STORAGE_TYPES_H_

