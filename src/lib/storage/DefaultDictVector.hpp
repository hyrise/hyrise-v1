#pragma once

#include <vector>
#include <boost/dynamic_bitset.hpp>

#include <storage/BaseAttributeVector.h>
#include <storage/BaseAllocatedAttributeVector.h>
#include <memory/StrategizedAllocator.h>
#include <memory/MallocStrategy.h>
#include <helper/types.h>

namespace {

// stuct storing information about one column
template <typename T>
struct DefaultDictColumn {
  DefaultDictColumn() = default;
  DefaultDictColumn (const DefaultDictColumn& other) :
    defaultValue(other.defaultValue), defaultValueIsSet(other.defaultValueIsSet),
    default_bit_vector(other.default_bit_vector),
    exception_positions(other.exception_positions),
    exception_values(other.exception_values)
  { }

  T defaultValue;
  bool defaultValueIsSet;

  boost::dynamic_bitset<> default_bit_vector;
  std::vector< size_t >   exception_positions;
  std::vector< T >        exception_values;
};

enum defaultDictValues {
  defaultDict_default_value     = true,
  defaultDict_non_default_value = false
};

}


// DefaultDictVector will store one default value per column and all values for rows, that do not store 
// default values.
//   default_bit_vector per column stores, which rows hold default/not default values.
//   For not default values, exception_positions stores, where to find the related value in exception_values.
// DefaultDictVector will use std allocator for data vectors, because hyrise allocator can not directly be
// used in std::vector
template <typename T, typename Allocator = StrategizedAllocator<T, MallocStrategy > >
class DefaultDictVector : public BaseAllocatedAttributeVector<DefaultDictVector<T, Allocator>, Allocator> {
private:
  std::vector< DefaultDictColumn<T> > _default_dict_table;
  
  size_t _columns;
  size_t _rows;
  
public:
  DefaultDictVector() = delete;
  explicit DefaultDictVector(size_t columns, size_t rows);
  ~DefaultDictVector() override;

  DefaultDictVector(const DefaultDictVector& other);

  void *data() override {
    throw std::runtime_error("Direct data access not allowed");
  }

  void setNumRows(size_t rows) override {
    throw std::runtime_error("Direct data access not allowed");
  }

  T default_value (size_t column) const {
    checkColumnAccess(column);
    return _default_dict_table[column].defaultValue;
  }


  inline T get(size_t column, size_t row) const override;
  inline void set(size_t column, size_t row, T value) override;

  void reserve(size_t rows) override;
  void resize(size_t rows) override;

  inline uint64_t capacity() override {
    if (_columns == 0)
      return 0;
    return _default_dict_table[0].default_bit_vector.size();
  }

  void clear() override;

  size_t size() override {
    return _rows;
  }

  std::shared_ptr<BaseAttributeVector<T>> copy() override;

  void rewriteColumn(const size_t column, const size_t bits) override { }


private:
  inline void checkAccess(const size_t& column, const size_t& row) const {
#ifdef EXPENSIVE_ASSERTIONS
    checkColumnAccess(column);
    if (row >= _rows) {
      throw std::out_of_range("Trying to access row '"
                              + std::to_string(row) + "' where only '"
                              + std::to_string(_rows) + "' available");
    }
#endif
  }

inline void checkColumnAccess(const size_t& column) const {
#ifdef EXPENSIVE_ASSERTIONS
    if (column >= _columns) {
      throw std::out_of_range("Trying to access column '"
                              + std::to_string(column) + "' where only '"
                              + std::to_string(_columns) + "' available");
    }
#endif
  }
};



template <typename T, typename Allocator>
DefaultDictVector<T, Allocator>::DefaultDictVector(size_t columns, size_t rows) :
  _columns(columns),
  _rows(0) {

  // create data structure for each column
  // default values are not set here, they will be set on first write per column
  DefaultDictColumn<T> column;
  for (size_t c = 0; c<_columns; ++c) {
    column.defaultValueIsSet = false;
    column.defaultValue = T(0);
    column.default_bit_vector.resize(rows, true);
    _default_dict_table.push_back(column);
  }
}

template <typename T, typename Allocator>
DefaultDictVector<T, Allocator>::~DefaultDictVector() {
}


template <typename T, typename Allocator>
DefaultDictVector<T, Allocator>::DefaultDictVector(const DefaultDictVector& other) : 
  _default_dict_table(other._default_dict_table),
  _columns(other._columns),
  _rows(other._rows)
{
}

template <typename T, typename Allocator>
void DefaultDictVector<T, Allocator>::clear() {
  for (DefaultDictColumn<T>& column: _default_dict_table) {
    column.defaultValueIsSet = false;
    column.defaultValue = T(0);
    column.default_bit_vector.clear();
    column.exception_positions.clear();
    column.exception_values.clear();
  }

  _rows = 0;
}

template <typename T, typename Allocator>
std::shared_ptr<BaseAttributeVector<T>> DefaultDictVector<T, Allocator>::copy() {
  DefaultDictVector<T, Allocator> *copy = new DefaultDictVector<T, Allocator>(*this);
  return std::shared_ptr<BaseAttributeVector<T>>(copy);
}

template <typename T, typename Allocator>
void DefaultDictVector<T, Allocator>::reserve(size_t rows) {
  // reserve will never shrink the size of the Vector
  if (_default_dict_table[0].default_bit_vector.size() >= rows)
    return;

  for (DefaultDictColumn<T>& column: _default_dict_table)
    column.default_bit_vector.resize(rows, true);
}

template <typename T, typename Allocator>
void DefaultDictVector<T, Allocator>::resize(size_t rows) {
  reserve(rows);
  _rows = rows;
}

template <typename T, typename Allocator>
inline T DefaultDictVector<T, Allocator>::get(size_t column, size_t row) const {
  checkAccess(column, row);
  // first check if requested field stores the default value
  if(_default_dict_table[column].default_bit_vector[row] == defaultDict_default_value) {
    return _default_dict_table[column].defaultValue;
  }
  // if not, search the specified row in the exceptions list
  auto exception_position = std::lower_bound(
    _default_dict_table[column].exception_positions.begin(),
    _default_dict_table[column].exception_positions.end(),
    row);
  // get the exception value at determined position
  return _default_dict_table[column].exception_values[
    ( exception_position - _default_dict_table[column].exception_positions.begin() )];
}

template <typename T, typename Allocator>
inline void DefaultDictVector<T, Allocator>::set(size_t column, size_t row, T value) {
  checkAccess(column,row);
  // create some handy shortcuts for current column
  auto& def_bits(_default_dict_table[column].default_bit_vector);
  auto& ex_pos(_default_dict_table[column].exception_positions);
  auto& ex_val(_default_dict_table[column].exception_values);
  T& def_val(_default_dict_table[column].defaultValue);

  // the first call of set on a column will define its default value
  if (! _default_dict_table[column].defaultValueIsSet) {
    _default_dict_table[column].defaultValueIsSet = true;
    def_val = value;
  }

  // nothing to do, if new and old value are default values
  if ((def_bits[row] == defaultDict_default_value) && (value == def_val))
    return;

  // get position in exception list
  // what to do at 'pos' depends whether old or new value is default value
  size_t pos = std::lower_bound(ex_pos.begin(),ex_pos.end(),row) - ex_pos.begin();

  if (def_bits[row] == defaultDict_default_value) {
    // new value is not default value but old value was
    // so insert a new exception pos/value
    def_bits[row] = defaultDict_non_default_value;
    auto ex_it = ex_pos.begin();
    ex_pos.insert(ex_it+pos,row);
    auto val_it = ex_val.begin();
    ex_val.insert(val_it+pos,value);
  } else {
    // old value was not default
    if (value == def_val) {
      // delete exception entry, if new value is default
      def_bits[row] = defaultDict_default_value;
      ex_pos.erase(ex_pos.begin()+pos);
      ex_val.erase(ex_val.begin()+pos);
    } else {
      ex_val[pos] = value;
    }
  }
}
