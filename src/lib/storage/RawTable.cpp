// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "RawTable.h"

#include <cassert>
#include <iostream>

#include "storage/meta_storage.h"
#include "storage/ColumnMetadata.h"

namespace hyrise { namespace storage { namespace rawtable {

RowHelper::RowHelper(const metadata_vec_t& m) : _m(m) {
  _tempData.resize(m.size(), nullptr);
}

RowHelper::~RowHelper() {
  reset();
}

void RowHelper::reset() {
  for(auto& d: _tempData) {
    free(d);
    d = nullptr;
  }
}

byte* RowHelper::build() const {
  size_t width = sizeof(record_header);
  for(size_t i=0; i < _m.size(); ++i) {
    if (types::isCompatible(_m[i].getType(),StringType))
      width += *(uint16_t*) _tempData[i] + 2; // string length in bytes plus length var
    else
      width += 8;
  }
  byte * data = (byte*) malloc( width );
  record_header *header = (record_header*) data;
  header->width = width;

  // Copy the complete data based on the simplification of data types
  byte *mov = data + sizeof(record_header);
  for(size_t i=0; i < _tempData.size(); ++i) {
    if (types::isCompatible(_m[i].getType(),StringType)) {
      memcpy(mov, _tempData[i], 2);
      memcpy(mov+2, _tempData[i]+2, *(uint16_t*) mov);
      mov += 2 + *(uint16_t*) mov;
    } else if (_m[i].getType() == FloatType) {
      memcpy(mov, _tempData[i], 4);
      mov += 4;
    } else {
      memcpy(mov, _tempData[i], 8);
      mov += 8;
    }
  }
  return data;
}

template<>
std::string RowHelper::convert(const byte* d, DataType t) {
  std::string result((char*) d + 2, *((uint16_t*) d));
  return result;
}

template<>
void RowHelper::set(size_t index, std::string val) {
  byte* tmp = (byte*) malloc(2 + val.size());
  memcpy(tmp+2, (byte*) val.c_str(), val.size());
  *((unsigned short*) tmp) = static_cast<uint16_t>(val.size());
  assert(_tempData[index] == nullptr);
  _tempData[index] = tmp;
}

} // namespace rawtable

RawTable::RawTable(const metadata_vec_t& m,
                   size_t initial_size) : _metadata(m), _width(m.size()), _size(0) {

  // Simple memory allocation,
  //TODO this should use the defined allocator strategy
  _data = (byte*) calloc(1024*1024, 1);
  _endOfData = _data;
  _endOfStorage = _data + 1024*1024;
}

RawTable::~RawTable() {
  free(_data);
}

size_t RawTable::size() const { return _size; };

size_t RawTable::columnCount() const { return _width; }

void RawTable::reserve(const size_t nr_of_values) {}

void RawTable::resize(const size_t nr_of_values) {}


const ColumnMetadata& RawTable::metadataAt(const size_t column, const size_t row, 
                                           const table_id_t table_id) const {
  return _metadata.at(column);
}


unsigned RawTable::partitionCount() const {
  // sh: reduced to zero, prevents PointerCalculator from updating fields
  return 0;
}

table_id_t RawTable::subtableCount() const {
  return 1;
}

atable_ptr_t RawTable::copy() const {
  return nullptr;
}

RawTable::byte* RawTable::computePosition(const size_t& column, const size_t& row) const {
  if (column > _metadata.size() ) {
    throw std::out_of_range("Column out of range in getValue()");
  }
  byte* tuple = getRow(row);
  tuple += sizeof(rawtable::record_header);
  for(size_t i=0; i < column; ++i) {
    if (types::isCompatible(_metadata[i].getType(),StringType)) {
      tuple += 2 /* size of the length */ + *((unsigned short*) tuple);
    } else {
      tuple += 8;
    }
  }
  return tuple;
}

template <typename T>
std::string to_string(T val) {
  return std::to_string(val);
}

template <>
std::string to_string(const std::string& val) { return val; }
template <>
std::string to_string(std::string& val) { return val; }
template <>
std::string to_string(std::string val) { return val; }

struct convert_to_string_functor {
  typedef std::string value_type;
  const RawTable& _table;
  const size_t& _col;
  const size_t& _row;

  convert_to_string_functor(const RawTable& table,
                            const size_t& col,
                            const size_t& row) :
      _table(table), _col(col), _row(row) {};

  template<typename R>
  std::string operator()() {
    return to_string(_table.template getValue<R>(_col, _row));
  }
};

std::string RawTable::printValue(const size_t col, const size_t row) const {
  type_switch<hyrise_basic_types> ts;
  convert_to_string_functor f(*this, col, row);
  return ts(_metadata.at(col).getType(), f);
}

RawTable::byte* RawTable::getRow(size_t index) const {
  if (index >= _size) throw std::out_of_range("Index out of range for getRow()");
  byte *data = _data;
  for(size_t i=0; i < index; ++i)
    data += ((rawtable::record_header*) data)->width;
  return data;
}

void RawTable::appendRow(byte* tuple) {
  size_t width  = ((rawtable::record_header*) tuple)->width;
  if ((_endOfData + width) > _endOfStorage) {
    size_t amount = ((_endOfData + width) - _data) * 2;
    size_t dist = _endOfData - _data;
    // TODO this should use the allocator strategy
    _data = (byte*) realloc(_data, amount);
    _endOfData = _data + dist;
    _endOfStorage = _data + amount;
  }

  memcpy(_endOfData, tuple, width);
  _endOfData += width;
  _size++;
}

struct type_func {
  typedef void value_type;

  const atable_ptr_t& _source;
  rawtable::RowHelper& _rh;
  const size_t& _col;
  const size_t& _row;

  type_func(const atable_ptr_t& source,
            rawtable::RowHelper& rh,
            const size_t& column,
            const size_t& row) : _source(source), _rh(rh), _col(column), _row(row) {}

  template<typename R>
  void operator()() {
    _rh.set<R>(_col, _source->getValue<R>(_col, _row));
  }
};

void RawTable::appendRows(const atable_ptr_t& rows) {
  type_switch<hyrise_basic_types> ts;
  for(size_t row=0; row < rows->size(); ++row) {
    rawtable::RowHelper rh(_metadata);
    for(size_t column=0; column < _metadata.size(); ++column) {
      type_func tf(rows, rh, column, row);
      ts(rows->typeOfColumn(column), tf);
    }
    std::unique_ptr<byte, void (*)(void *)> data(rh.build(), &std::free);
    appendRow(data.get());
  }
}

void RawTable::debugStructure(size_t level) const {
  std::cout << std::string(level, '\t') << "RawTable " << this << std::endl;
}

} } // namespace hyrise::storage
