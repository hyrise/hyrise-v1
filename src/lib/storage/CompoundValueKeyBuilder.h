// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include "storage/storage_types.h"
#include "storage/AbstractTable.h"

namespace hyrise {
namespace storage {

class CompoundValueKeyBuilder {
 private:
  compound_value_key_t _key;
  int _offset;

 public:
  CompoundValueKeyBuilder() : _offset(0) { memset(&_key[0], 0, sizeof(compound_value_key_t)); }

  template <typename T>
  void add(T value) {
    throw std::runtime_error("not yet supported for compound keys");
  }

  void add(hyrise_int_t value) {
    assert(__BYTE_ORDER__ ==
           __ORDER_LITTLE_ENDIAN__);  // http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
    assert(sizeof(hyrise_int_t) == 8);

    hyrise_int_t value_s = value ^ (hyrise_int_t)0x8000000000000000;  // toggle MSB so that -x < x when binary compared
    assert(abs(value_s) == abs(value));
    value = value_s;


    if (_offset + sizeof(hyrise_int_t) > compound_value_key_size) {
      throw std::runtime_error("Maximum size of compound key exceeded");
    }
    value = __builtin_bswap64(value);
    memcpy(&_key[_offset], &value, sizeof(hyrise_int_t));
    _offset += sizeof(hyrise_int_t);
  }

  void add(hyrise_float_t value) {
    throw std::runtime_error("Floats are not yet supported for compound keys");
    // see https://stackoverflow.com/questions/3103504 for details
  }

  void add(hyrise_string_t value) {
    // currently, this does not support variable length strings
    // instead, we truncate/pad all strings to internal_string_length characters
    constexpr long unsigned int internal_string_length = 20;

    assert(value.length() < internal_string_length);

    if (_offset + internal_string_length > compound_value_key_size) {
      throw std::runtime_error("Maximum size of compound key exceeded");
    }
    int used_string_length = std::min(value.length(), internal_string_length);
    memcpy(&_key[_offset], value.c_str(), used_string_length);
    _offset += used_string_length;
    memset(&_key[_offset], 0, internal_string_length - used_string_length);
  }

  compound_value_key_t get() {
    memset(&_key[_offset], 0, compound_value_key_size - _offset);
    return _key;
  }
  compound_value_key_t get_upperbound() {
    memset(&_key[_offset], 255, compound_value_key_size - _offset);
    return _key;
  }
};

class AddValueToCompoundKeyFunctor {
 private:
  CompoundValueKeyBuilder& _builder;
  const AbstractTable* _table;
  pos_t _row;
  field_t _column;

 public:
  typedef bool value_type;

  AddValueToCompoundKeyFunctor(CompoundValueKeyBuilder& builder, const AbstractTable* table, pos_t row, field_t column)
      : _builder(builder), _table(table), _row(row), _column(column) {}

  template <typename ValueType>
  value_type operator()() {
    ValueType value = _table->getValue<ValueType>(_column, _row);
    _builder.add(value);
    return true;
  }
};
}
}
