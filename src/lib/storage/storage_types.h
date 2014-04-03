// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <ostream>
#include <iostream>
#include <cassert>
#include <array>
#include <memory>


#define STORAGE_XSTR(x) STORAGE_STR(x)
#define STORAGE_STR(x) #x
#define STORAGE_DEBUG_WHERE_WHAT(file, line) std::string(#file ":" #line)
#define STORAGE_NOT_IMPLEMENTED(class, method) \
  throw std::runtime_error(#class "(" STORAGE_XSTR(__FILE__) ":" STORAGE_XSTR(__LINE__) ") does not " #method);

/**
 * These Types reflect the basic types that are available in
 * HYRISE. However, they do not necessarily reflect the underlying
 * type. E.g. a no_dict_integer is a normal integer type, but
 * different when it comes to using the dictionary. And this
 * information is preserverd in the original type
 */
typedef enum {
  IntegerType = 0,
  FloatType,
  StringType,
  // Unordered Delta Types
  IntegerTypeDelta,
  FloatTypeDelta,
  StringTypeDelta,

  // ConcurrentTypes
  IntegerTypeDeltaConcurrent,
  FloatTypeDeltaConcurrent,
  StringTypeDeltaConcurrent,

  // No Dict types
  IntegerNoDictType,
  FloatNoDictType
} DataType;

namespace hyrise {
namespace types {

namespace detail {
const int num_basic_types = 3;
}

typedef std::string type_t;

const std::string integer_name = "INTEGER";
const std::string integer_name_main = "INTEGER_MAIN";
const std::string integer_name_conc = "INTEGER_DELTA_CONC";
const std::string no_dict_integer_name = "INTEGER_NO_DICT";
const std::string float_name = "FLOAT";
const std::string float_name_main = "FLOAT_MAIN";
const std::string float_name_conc = "FLOAT_DELTA_CONC";
const std::string no_dict_float_name = "FLOAT_NO_DICT";
const std::string string_name = "STRING";
const std::string string_name_conc = "STRING_DELTA_CONC";
const std::string string_name_main = "STRING_MAIN";


/*
 * maps the type to its concurrent version
 */
inline DataType getConcurrentType(DataType t) {
  switch (t) {
    case IntegerType:
    case IntegerTypeDelta:
    case IntegerTypeDeltaConcurrent:
      return IntegerTypeDeltaConcurrent;
    case FloatType:
    case FloatTypeDelta:
    case FloatTypeDeltaConcurrent:
      return FloatTypeDeltaConcurrent;
    case StringType:
    case StringTypeDelta:
    case StringTypeDeltaConcurrent:
      return StringTypeDeltaConcurrent;
    case IntegerNoDictType:
      return IntegerNoDictType;
    case FloatNoDictType:
      return FloatNoDictType;
  }
  throw std::runtime_error("No concurrent mapping for datatype");
}

inline DataType getOrderedType(DataType t) {
  switch (t) {
    case IntegerType:
    case IntegerTypeDelta:
    case IntegerTypeDeltaConcurrent:
    case IntegerNoDictType:
      return IntegerType;
    case FloatType:
    case FloatTypeDelta:
    case FloatTypeDeltaConcurrent:
    case FloatNoDictType:
      return FloatType;
    case StringType:
    case StringTypeDelta:
    case StringTypeDeltaConcurrent:
      return StringType;
  }
  throw std::runtime_error("No ordered mapping for datatype");
}

inline DataType getUnorderedType(DataType t) {
  switch (t) {
    case IntegerType:
    case IntegerTypeDelta:
    case IntegerTypeDeltaConcurrent:
      return IntegerTypeDelta;
    case FloatType:
    case FloatTypeDelta:
    case FloatTypeDeltaConcurrent:
      return FloatTypeDelta;
    case StringType:
    case StringTypeDelta:
    case StringTypeDeltaConcurrent:
      return StringTypeDelta;
    case IntegerNoDictType:
      return IntegerNoDictType;
    case FloatNoDictType:
      return FloatNoDictType;
  }
  throw std::runtime_error("No unordered mapping for datatype");
}

inline bool isUnordered(DataType t) { return t > StringType; }

inline bool isDictionaryEncoded(DataType t) { return t >= IntegerNoDictType; }

inline bool isCompatible(DataType a, DataType b) {
  return (a % detail::num_basic_types) == (b % detail::num_basic_types);
}
}
}

typedef int64_t hyrise_int_t;
typedef int32_t hyrise_int32_t;
typedef float hyrise_float_t;
typedef std::string hyrise_string_t;

typedef uint32_t value_id_t;
typedef uint8_t table_id_t;

typedef size_t pos_t;
typedef size_t field_t;

typedef std::string field_name_t;
typedef std::vector<field_name_t> field_name_list_t;

typedef std::vector<pos_t> pos_list_t;
typedef std::vector<field_t> field_list_t;

constexpr int compound_value_key_size = 40;
typedef std::array<uint8_t, compound_value_key_size> compound_value_key_t;
typedef uint64_t compound_valueid_key_t;

/*
  This is the ValueID class used to store important information
*/
class ValueId {
 public:
  value_id_t valueId;
  table_id_t table;

  ValueId() : valueId(0), table(0) {}
  ValueId(value_id_t _valueId, table_id_t _table) : valueId(_valueId), table(_table) {}


  bool operator==(ValueId& o) { return valueId == o.valueId && table == o.table; }

  bool operator!=(ValueId& o) { return valueId != o.valueId && table == o.table; }
  bool operator<(const ValueId& o) const {
    if (table != o.table)
      throw std::runtime_error("comparing value ids of different tables");
    return valueId < o.valueId;
  }

  bool operator>(const ValueId& o) const {
    if (table != o.table)
      throw std::runtime_error("comparing value ids of different tables");
    return valueId > o.valueId;
  }

  friend inline std::ostream& operator<<(std::ostream& os, const ValueId& v) {
    os << "<ValueID v:" << v.valueId << " t:" << (int)v.table << ">";
    return os;
  }
};

typedef std::vector<ValueId> ValueIdList;

namespace hyrise {
namespace storage {

class ColumnMetadata;
typedef std::vector<ColumnMetadata> metadata_list;
typedef std::vector<metadata_list*> compound_metadata_list;
typedef std::vector<ColumnMetadata> metadata_vec_t;


class PositionRange {
 protected:
  std::shared_ptr<pos_list_t> _vector;
  pos_list_t::const_iterator _cbegin;
  pos_list_t::const_iterator _cend;
  bool _sorted;


 public:
  PositionRange(pos_list_t::const_iterator cbegin,
                pos_list_t::const_iterator cend,
                bool sorted = false,
                std::shared_ptr<pos_list_t> vector = nullptr)
      : _vector(vector), _cbegin(cbegin), _cend(cend), _sorted(sorted) {
    assert(std::distance(_cbegin, _cend) >= 0);
  }


  pos_list_t::const_iterator cbegin() const { return _cbegin; }

  pos_list_t::const_iterator cend() const { return _cend; }

  size_t size() const { return std::distance(_cbegin, _cend); }

  bool isSorted() const { return _sorted; }
};
}
}  // namespace hyrise::storage
