// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_RAWTABLE_H_
#define SRC_LIB_STORAGE_RAWTABLE_H_

#include <stdexcept>
#include <vector>

#include <stdint.h>

#include "storage/storage_types.h"
#include "storage/meta_storage.h"
#include "storage/AbstractTable.h"
#include "storage/AbstractAllocatedTable.h"
#include "storage/ColumnMetadata.h"




namespace hyrise { namespace storage { namespace rawtable {

struct record_header {
  size_t width;
};

struct RowHelper {
  typedef unsigned char byte;

  record_header _header;
  const metadata_vec_t& _m;
  std::vector<byte*> _tempData;

  void reset() {
    _header = {0};
    for(auto d: _tempData)
      free(d);
    _tempData.clear();
  }

  RowHelper(const metadata_vec_t& m) : _m(m) {
    _tempData.resize(m.size());
  }

  /**
   * Based on the value type add an intermediate memory location and
   * store this until we finalize the record. This method has some
   * special handling for string types
   */
  template<typename T>
  void set(size_t index, T val) {
    byte* tmp = (byte*) malloc(sizeof(T));
    memcpy(tmp, (byte*) &val, sizeof(T));
    _tempData[index] = tmp;
  }

  /**
   * This helper method builds the actual data stream that represents
   * the row as a binary block. All fixed length values are encoded
   * using 8 bytes, while strings are encoded using a two byte lenght
   * identifier followed by the actual string.
   *
   * The return value is allocate in this method and memory handling
   * is returned to the caller
   */
  byte* build() const {
    size_t width = sizeof(record_header);
    for(size_t i=0; i < _m.size(); ++i) {
      if (_m[i].getType() == StringType)
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
      if (_m[i].getType() == StringType) {
        memcpy(mov, _tempData[i], 2);
        memcpy(mov+2, _tempData[i]+2, *(uint16_t*) mov);
        mov += 2 + *(uint16_t*) mov;
      } else {
        memcpy(mov, _tempData[i], 8);
        mov += 8;
      }
    }
    return data;
  }

  template<typename T>
  static T convert(const byte* data, DataType t) {
    T result;
    memcpy(&result, data, sizeof(T));
    return result;
  }
};

template<>
void RowHelper::set(size_t index, std::string val);

template<>
std::string RowHelper::convert(const RowHelper::byte *d, DataType t);



}}}

namespace hyrise {
template <typename T>
std::string to_string(T val) {
  return std::to_string(val);
}

template <>
std::string to_string(const std::string& val);
template <>
std::string to_string(std::string& val);
template <>
std::string to_string(std::string val);
}



ALLOC_CLASS(RawTable) {
  typedef RawTable<Strategy, Allocator> this_type;
  typedef unsigned char byte;
  typedef metadata_vec_t MetadataVector;

  //* Table structure
  MetadataVector _metadata;

  //* Number of columns
  size_t _width;

  //* Number of tuples
  size_t _size;

  // Row wise offset vector that helps to pinpoint the 
  // correct memory location for each row
  std::vector<size_t> _offsets;

  // Pointer to the main data
  byte *_data;
  byte *_endOfData;
  byte *_endOfStorage;

public:

  RawTable(const metadata_vec_t& m,
           size_t initial_size = 0) : _metadata(m), _width(m.size()), _size(0) {

    // Simple memory allocation,
    //TODO this should use the defined allocator strategy
    _data = (byte*) calloc(1024*1024, 1);
    _endOfData = _data;
    _endOfStorage = _data + 1024*1024;
  }

  virtual ~RawTable() {
    free(_data);
  }

  size_t size() const { return _size; };

  size_t columnCount() const { return _width; }

  void reserve(const size_t nr_of_values) {}

  void resize(const size_t nr_of_values) {}


  virtual const ColumnMetadata *metadataAt(const size_t column, const size_t row = 0, 
                                     const table_id_t table_id = 0) const {
    return &(_metadata.at(column));
  }

  unsigned sliceCount() const {
    // sh: reduced to zero, prevents PointerCalculator from updating fields
    return 0;
  }

  virtual table_id_t subtableCount() const {
    return 1;
  }

  virtual hyrise::storage::atable_ptr_t copy() const {
    return nullptr;
  }

  virtual size_t getSliceForColumn(const size_t column) const {
    return 0;
  }

  virtual size_t getOffsetInSlice(const size_t column) const {
    return 0;
  }

  byte* computePosition(const size_t& column, const size_t& row) const {
    if (column > _metadata.size() ) {
      throw std::out_of_range("Column out of range in getValue()");
    }
    byte* tuple = getRow(row);
    tuple += sizeof(hyrise::storage::rawtable::record_header);
    for(size_t i=0; i < column; ++i) {
      if (_metadata[i].getType() == StringType) {
        tuple += 2 /* size of the length */ + *((unsigned short*) tuple);
      } else {
        tuple += 8;
      }
    }
    return tuple;
  }

  template <typename T>
  T getValue(const size_t column, const size_t row) const {
    const byte* tuple = computePosition(column, row);
    return hyrise::storage::rawtable::RowHelper::convert<T>(tuple, _metadata[column].getType());
  }


  template <typename T>
  void setValue(const size_t column, const size_t row, const T& value) {
    byte* tuple = computePosition(column, row);
    memcpy(tuple, (byte*) &value, sizeof(T)); // All values except for strings are written with 8 bytes
  }

  void setValue(const size_t, const size_t, const std::string&) {
    throw std::runtime_error("Setting of strings is not implemented");
  }

  template <typename T>
  struct convert_to_string_functor {
    typedef std::string value_type;
    const T& _table;
    const size_t& _col;
    const size_t& _row;

    convert_to_string_functor(const T& table,
                              const size_t& col,
                              const size_t& row) :
        _table(table), _col(col), _row(row) {};

    template<typename R>
    std::string operator()() {
      return hyrise::to_string(_table.template getValue<R>(_col, _row));
    }
  };

  std::string printValue(const size_t col, const size_t row) const {
    hyrise::storage::type_switch<hyrise_basic_types> ts;
    convert_to_string_functor<this_type> f(*this, col, row);
    return ts(_metadata.at(col).getType(), f);
  }

  /**
   * Retrieve the row from the table based on the index, this will
   * return a pointer to the actual values, which cannot be modified
   * and the ownership will not be transferred.
   */
  byte* getRow(size_t index) const {
    if (index >= _size) throw std::out_of_range("Index out of range for getRow()");
    byte *data = _data;
    for(size_t i=0; i < index; ++i)
      data += ((hyrise::storage::rawtable::record_header*) data)->width;
    return data;
  }

  /**
   * Append a new row to the end of the storage, in case this means we
   * do not have sufficient space we will reallocate space
   * accordingly.
   */
  void appendRow(byte* tuple) {
    size_t width  = ((hyrise::storage::rawtable::record_header*) tuple)->width;
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

    const hyrise::storage::atable_ptr_t& _source;
    hyrise::storage::rawtable::RowHelper& _rh;
    const size_t& _col;
    const size_t& _row;

    type_func(const hyrise::storage::atable_ptr_t& source,
              hyrise::storage::rawtable::RowHelper& rh,
              const size_t& column,
              const size_t& row) : _source(source), _rh(rh), _col(column), _row(row) {}

    template<typename R>
    void operator()() {
      _rh.set<R>(_col, _source->getValue<R>(_col, _row));
    }
  };

  void appendRows(const hyrise::storage::atable_ptr_t& rows) {
    hyrise::storage::type_switch<hyrise_basic_types> ts;
    for(size_t row=0; row < rows->size(); ++row) {
      hyrise::storage::rawtable::RowHelper rh(_metadata);
      for(size_t column=0; column < _metadata.size(); ++column) {
        type_func tf(rows, rh, column, row);
        ts(rows->typeOfColumn(column), tf);
      }
      std::unique_ptr<unsigned char> data(rh.build());
      appendRow(data.get());
    }
  }

  virtual void debugStructure(size_t level=0) const {
    std::cout << std::string(level, '\t') << "RawTable " << this << std::endl;
  }


  
  ////////////////////////////////////////////////////////////////////////////////////////
  // Disabled Methodsw 
  virtual hyrise::storage::atable_ptr_t copy_structure(const field_list_t *fields = nullptr, 
                                                        const bool reuse_dict = false, 
                                                        const size_t initial_size = 0, 
                                                        const bool with_containers = true, 
                                                        const bool compressed = false) const {
    STORAGE_NOT_IMPLEMENTED(RawTable, copy_structure());
  }

  virtual hyrise::storage::atable_ptr_t copy_structure_modifiable(const field_list_t *fields = nullptr, 
                                                                   const size_t initial_size = 0, 
                                                                   const bool with_containers = true) const {
    STORAGE_NOT_IMPLEMENTED(RawTable, copy_structure_modifiable());
  }


  ValueId getValueId(const size_t column, const size_t row) const { 
    STORAGE_NOT_IMPLEMENTED(RawTable, getValueId());
  }
  
  void setValueId(const size_t column, const size_t row, const ValueId valueId) {
    STORAGE_NOT_IMPLEMENTED(RawTable, setValueId());
  }

  virtual void *atSlice(const size_t slice, const size_t row) const {
    STORAGE_NOT_IMPLEMENTED(RawTable, atSlice());
  }

  virtual size_t getSliceWidth(const size_t slice) const {
    STORAGE_NOT_IMPLEMENTED(RawTable, getSliceWidth());
  }

  virtual const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, 
                                                          const size_t row = 0, 
                                                          const table_id_t table_id = 0, 
                                                          const bool of_delta = false) const { 
    STORAGE_NOT_IMPLEMENTED(RawTable, getSliceWidth());
  }

  virtual const AbstractTable::SharedDictionaryPtr& dictionaryByTableId(const size_t column, 
                                                                 const table_id_t table_id) const { 
    STORAGE_NOT_IMPLEMENTED(RawTable, dictionaryByTableId());
  }

  virtual void setDictionaryAt(AbstractTable::SharedDictionaryPtr dict, 
                               const size_t column, const size_t row = 0, const table_id_t table_id = 0) {
    STORAGE_NOT_IMPLEMENTED(RawTable, dictionaryAt());
  }


};
#endif // SRC_LIB_STORAGE_RAWTABLE_H_
