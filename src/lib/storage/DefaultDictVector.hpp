#pragma once

#include <vector>
#include <iterator>
#include <boost/dynamic_bitset.hpp>

#include <storage/BaseAttributeVector.h>
#include <storage/BaseAllocatedAttributeVector.h>
#include <memory/StrategizedAllocator.h>
#include <memory/MallocStrategy.h>
#include <helper/types.h>

typedef hyrise::storage::pos_t pos_t;

// Template classes for iterators
template <typename T> class DefaultDictVectorIterator;
template <typename T> class DefaultDictVectorBufferedIterator;

// use malloc to allocate memory
template <typename T, typename Allocator = StrategizedAllocator<T, MallocStrategy > >
class DefaultDictVector : public BaseAllocatedAttributeVector<DefaultDictVector<T, Allocator>, Allocator> {
private:
  T _default_value;

  boost::dynamic_bitset<>  _default_bit_vector;
  std::vector< pos_t >     _exception_positions;
  std::vector< T >         _exception_values;
  
  size_t _rows;

  inline T getValueAt(pos_t position) const {
    if (_default_bit_vector[position]) {
      return _default_value;
    }
    else {
      // binary search on exception list
      auto low = std::lower_bound(
        _exception_positions.begin(),
        _exception_positions.end(),
        position);
        // assert (*low == position);
        return _exception_values[
          ( low - _exception_positions.begin() )];
    }
  }
  
public:    
  typedef DefaultDictVectorIterator<T> Iterator;
  typedef DefaultDictVectorBufferedIterator<T> BufferedIterator;
  
  explicit DefaultDictVector(T _default_value);
  ~DefaultDictVector();
  
  Iterator begin() {
    return Iterator(
      this->_default_bit_vector,
      this->_exception_positions,
      this->_exception_values,
      this->_default_value);
  };
  Iterator end() {
    return Iterator(
      this->_default_bit_vector,
      this->_exception_positions,
      this->_exception_values,
      this->_default_value,
      this->size());
  }
  Iterator iterator(pos_t position) {
    return Iterator(
      this->_default_bit_vector,
      this->_exception_positions,
      this->_exception_values,
      this->_default_value,
      position);
  }

  void *data() {
    return nullptr;
  }

  void setNumRows(size_t) {
    throw std::bad_exception();
  }
  
  size_t allocated_bytes();
  
  void push_back(T value);
  
  T operator[] (size_t n) {
    return getValueAt(n);
  }

  T default_value () {
    return _default_value;
  }


  // BaseAttributeVector interface
  inline T get(size_t column, size_t row) const {
    if (column>0)
      throw std::out_of_range("Trying to access an other column then 0 where only one column is supported until now..");
    return getValueAt(row);
  }

  inline void set(size_t column, size_t row, T value) {
    if (column>0)
      throw std::out_of_range("Trying to access an other column then 0 where only one column is supported until now..");
    if (! row==_rows+1) {
      throw std::out_of_range("Trying to access an other column then 0 where only one column is supported until now..");
    }
       
    push_back(value);
  }

  void reserve(size_t) { }

  void resize(size_t) { }

  inline uint64_t capacity() {
    return _default_bit_vector.size();
  }

  void clear();

  size_t size() {
    return _rows;
  }

  std::shared_ptr<BaseAttributeVector<T>> copy();

  void rewriteColumn(const size_t column, const size_t bits) { }
};



template <typename T, typename Allocator>
DefaultDictVector<T, Allocator>::DefaultDictVector(T default_value) :
  _default_value(default_value), 
  _rows(0) {
}

template <typename T, typename Allocator>
DefaultDictVector<T, Allocator>::~DefaultDictVector() {
  return;
}

template <typename T, typename Allocator>
void DefaultDictVector<T, Allocator>::push_back (T value) {
  if (value == this->_default_value) {
    _default_bit_vector.push_back(true);
  }
  else {
    _default_bit_vector.push_back(false);
    _exception_positions.push_back( this->_rows );
    _exception_values.push_back( value );
  }
  ++this->_rows;
}  

template <typename T, typename Allocator>
size_t DefaultDictVector<T, Allocator>::allocated_bytes() {
  return _default_bit_vector.num_blocks()*_default_bit_vector.bits_per_block/8
       + _exception_positions.capacity()*sizeof(pos_t)
       + _exception_values.capacity()*sizeof(T);
}

template <typename T, typename Allocator>
void DefaultDictVector<T, Allocator>::clear() {
  _exception_positions.clear();
  _exception_values.clear();
  _default_bit_vector.clear();

  _rows=0;
}

template <typename T, typename Allocator>
std::shared_ptr<BaseAttributeVector<T>> DefaultDictVector<T, Allocator>::copy() {
  throw std::bad_exception();
  //auto copy = std::make_shared<DefaultDictVector<T, Allocator>>(_default_value);
  //copy->_rows = _rows;
}




template <typename T>
class DefaultDictVectorIterator : 
public std::iterator< std::input_iterator_tag, int>
{
  boost::dynamic_bitset<>& _default_bit_vector;
  std::vector< pos_t >&    _exception_positions;
  std::vector< T >&        _exception_values;

  T _default_value;
  pos_t _my_position;
  pos_t _pos_in_exception_list;

public:
  DefaultDictVectorIterator(
    boost::dynamic_bitset<>& default_bit_vector,
    std::vector<pos_t>&      exception_positions, 
    std::vector<T>&          exception_values,
    T                        default_value);

  DefaultDictVectorIterator(
    boost::dynamic_bitset<>& default_bit_vector,
    std::vector<pos_t>&      exception_positions, 
    std::vector<T>&          exception_values,
    T                        default_value,
    pos_t                    position);

  DefaultDictVectorIterator& operator++() {
    if (!_default_bit_vector[_my_position])
      ++_pos_in_exception_list;
    ++_my_position;
    return *this;
  }
  
  inline T& operator*() {
    if (_default_bit_vector[_my_position]) {
      return _default_value;
    } else {
      return _exception_values[_pos_in_exception_list];      
    }
  }
  
  bool operator==(const DefaultDictVectorIterator& rhs) const {
    return _my_position==rhs._my_position;
  }
  
  bool operator!=(const DefaultDictVectorIterator& rhs) const {
    return _my_position!=rhs._my_position;
  }

  bool operator<(const DefaultDictVectorIterator& rhs) const {
    return _my_position<rhs._my_position;
  }
  
  bool operator>(const DefaultDictVectorIterator& rhs) const {
    return _my_position>rhs._my_position;
  }

  bool operator<=(const DefaultDictVectorIterator& rhs) const {
    return _my_position<=rhs._my_position;
  }
  
  bool operator>=(const DefaultDictVectorIterator& rhs) const {
    return _my_position>=rhs._my_position;
  }
  DefaultDictVectorIterator operator+(pos_t position) const {
    return DefaultDictVectorIterator(
      this->_default_bit_vector,
      this->_exception_positions,
      this->_exception_values,
      this->_default_value,
      this->_my_position + position);
  }
  DefaultDictVectorIterator operator-(pos_t position) const {
    return DefaultDictVectorIterator(
      this->_default_bit_vector,
      this->_exception_positions,
      this->_exception_values,
      this->_default_value,
      this->_my_position - position);
  }
};



template <typename T>
DefaultDictVectorIterator<T>::DefaultDictVectorIterator (
  boost::dynamic_bitset<>& default_bit_vector,
  std::vector<pos_t>& exception_positions, 
  std::vector<T>&     exception_values,
  T default_value) :
    _default_bit_vector(default_bit_vector),
    _exception_positions(exception_positions),
    _exception_values(exception_values),
    _default_value(default_value),
    _my_position(0),
    _pos_in_exception_list(0)
{ }


template <typename T>
DefaultDictVectorIterator<T>::DefaultDictVectorIterator (
  boost::dynamic_bitset<>& default_bit_vector,
  std::vector<pos_t>& exception_positions, 
  std::vector<T>&     exception_values,
  T default_value,
  pos_t position) :
    _default_bit_vector(default_bit_vector),
    _exception_positions(exception_positions),
    _exception_values(exception_values),
    _default_value(default_value),
    _my_position(position),
    _pos_in_exception_list(0)
{
  if (_my_position and (_my_position < _default_bit_vector.size())) {
    _pos_in_exception_list = std::lower_bound(_exception_positions.begin(), 
        _exception_positions.end(), _my_position) - _exception_positions.begin(); 
  }
}

