#pragma once

#include <vector>
#include <iterator>
#include <boost/dynamic_bitset.hpp>

#include <storage/BaseAttributeVector.h>
#include <storage/BaseAllocatedAttributeVector.h>
#include <memory/StrategizedAllocator.h>
#include <memory/MallocStrategy.h>
#include <helper/types.h>

#include <iostream>


template <typename T>
struct DefaultDictColumn {
  T default_value;
  bool defaultValueIsSet;

  boost::dynamic_bitset<> default_bit_vector;
  std::vector< size_t >   exception_positions;
  std::vector< T >        exception_values;
};

enum defaultDictValues {
  defaultDict_default_value     = true,
  defaultDict_non_default_value = false
};


// use malloc to allocate memory
template <typename T, typename Allocator = StrategizedAllocator<T, MallocStrategy > >
class DefaultDictVector : public BaseAllocatedAttributeVector<DefaultDictVector<T, Allocator>, Allocator> {
private:
  std::vector< DefaultDictColumn<T> > _default_dict_table;
  
  size_t _columns;
  size_t _rows;
  
public:    
  explicit DefaultDictVector(size_t columns, size_t rows);
  ~DefaultDictVector();

  void *data() {
    throw std::runtime_error("Direct data access not allowed");
  }

  void setNumRows(size_t rows) {
    //_rows = rows;
    throw std::runtime_error("Direct data access not allowed");
  }
  
  size_t allocated_bytes();

  T default_value (size_t column) const {
    checkColumnAccess(column);
    return _default_dict_table[column].default_value;
  }


  inline T get(size_t column, size_t row) const {
    checkAccess(column, row);
    if(_default_dict_table[column].default_bit_vector[row]) {
      return _default_dict_table[column].default_value;
    }
    auto exception_position = std::lower_bound(
      _default_dict_table[column].exception_positions.begin(),
      _default_dict_table[column].exception_positions.end(),
      row);
    return _default_dict_table[column].exception_values[
      ( exception_position - _default_dict_table[column].exception_positions.begin() )];
  }


  inline void set(size_t column, size_t row, T value) {
    checkAccess(column,row);
    auto& def_bits(_default_dict_table[column].default_bit_vector);
    auto& ex_pos(_default_dict_table[column].exception_positions);
    auto& ex_val(_default_dict_table[column].exception_values);
    auto& def_val(_default_dict_table[column].default_value);

    if (! _default_dict_table[column].defaultValueIsSet) {
      _default_dict_table[column].defaultValueIsSet = true;
      def_val = value;
    }

    if ((def_bits[row]==defaultDict_default_value) and (value == def_val))
      return;

    size_t pos = std::lower_bound(ex_pos.begin(),ex_pos.end(),row) - ex_pos.begin();

    //value is not default value but old value was
    if (def_bits[row]) {
      def_bits[row] = defaultDict_non_default_value;
      auto ex_it = ex_pos.begin();
      ex_pos.insert(ex_it+pos,row);
      auto val_it = ex_val.begin();
      ex_val.insert(val_it+pos,value);
    } else {
      if (value == def_val) {
        def_bits[row] = defaultDict_default_value;
        ex_pos.erase(ex_pos.begin()+pos);
        ex_val.erase(ex_val.begin()+pos);
      } else {
        ex_val[pos] = value;
      }
    }
  }

  void reserve(size_t rows);

  void resize(size_t rows) {
    reserve(rows);
    _rows = rows;
  }

  inline uint64_t capacity() {
    if (!_columns)
      return 0;
    return _default_dict_table[0].default_bit_vector.size();
  }

  void clear();

  size_t size() {
    return _rows;
  }

  std::shared_ptr<BaseAttributeVector<T>> copy();

  void rewriteColumn(const size_t column, const size_t bits) { }


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

  DefaultDictColumn<T> column;
  for (size_t c = 0; c<_columns; ++c) {
    column.defaultValueIsSet = false;
    column.default_value = T(0);
    column.default_bit_vector.resize(rows, true);
    _default_dict_table.push_back(column);
  }
}

template <typename T, typename Allocator>
DefaultDictVector<T, Allocator>::~DefaultDictVector() {
}

template <typename T, typename Allocator>
void DefaultDictVector<T, Allocator>::clear() {
  for(size_t c = 0; c<_columns; ++c) {
    _default_dict_table[c].defaultValueIsSet = false;
    _default_dict_table[c].default_value = T(0);
    _default_dict_table[c].default_bit_vector.clear();
    _default_dict_table[c].exception_positions.clear();
    _default_dict_table[c].exception_values.clear();
  }

  _rows = 0;
}

template <typename T, typename Allocator>
std::shared_ptr<BaseAttributeVector<T>> DefaultDictVector<T, Allocator>::copy() {
  throw std::bad_exception();
  //auto copy = std::make_shared<DefaultDictVector<T, Allocator>>(_default_value);
  //copy->_rows = _rows;
}

template <typename T, typename Allocator>
void DefaultDictVector<T, Allocator>::reserve(size_t rows) {
  if (_default_dict_table[0].default_bit_vector.size() >= rows)
    return;

  for (size_t c=0; c<_columns; ++c)
    _default_dict_table[c].default_bit_vector.resize(rows, true);
}
