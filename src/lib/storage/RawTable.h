// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cassert>
#include <cstring>
#include <cstdint>

#include <stdexcept>
#include <vector>

#include "storage/storage_types.h"
#include "storage/AbstractTable.h"
#include "storage/ColumnMetadata.h"

namespace hyrise { namespace storage { namespace rawtable {

typedef unsigned char byte;

struct record_header {
  size_t width;
};

struct RowHelper {
  
  const metadata_vec_t& _m;
  std::vector<byte*> _tempData;

  RowHelper(const metadata_vec_t& m);
  ~RowHelper();
  /**
   * Based on the value type add an intermediate memory location and
   * store this until we finalize the record. This method has some
   * special handling for string types
   */
  template<typename T>
  void set(size_t index, T val) {
    byte* tmp = (byte*) malloc(sizeof(T));
    memcpy(tmp, (byte*) &val, sizeof(T));
    assert(_tempData[index] == nullptr);
    _tempData[index] = tmp;
  }

  void reset();
  
  /**
   * This helper method builds the actual data stream that represents
   * the row as a binary block. All fixed length values are encoded
   * using 8 bytes, while strings are encoded using a two byte lenght
   * identifier followed by the actual string.
   *
   * The return value is allocate in this method and memory handling
   * is returned to the caller
   */
  byte* build() const;

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
std::string RowHelper::convert(const byte *d, DataType t);

} // namespace rawtable

class RawTable : public AbstractTable {
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

  RawTable(const metadata_vec_t& m, size_t initial_size = 0);

  virtual ~RawTable();
  
  size_t size() const;

  size_t columnCount() const;

  void reserve(const size_t nr_of_values);

  void resize(const size_t nr_of_values);


  const ColumnMetadata& metadataAt(const size_t column_index, const size_t row_index = 0, const table_id_t table_id = 0) const override;
  
  unsigned partitionCount() const;

  virtual table_id_t subtableCount() const;
  
  virtual atable_ptr_t copy() const;

  byte* computePosition(const size_t& column, const size_t& row) const;

  template <typename T>
  T getValue(const size_t column, const size_t row) const {
    const byte* tuple = computePosition(column, row);
    return rawtable::RowHelper::convert<T>(tuple, _metadata[column].getType());
  }


  template <typename T>
  void setValue(const size_t column, const size_t row, const T& value) {
    byte* tuple = computePosition(column, row);
    memcpy(tuple, (byte*) &value, sizeof(T)); // All values except for strings are written with 8 bytes
  }

  void setValue(const size_t, const size_t, const std::string&) {
    throw std::runtime_error("Setting of strings is not implemented");
  }


  std::string printValue(const size_t col, const size_t row) const;

  /**
   * Retrieve the row from the table based on the index, this will
   * return a pointer to the actual values, which cannot be modified
   * and the ownership will not be transferred.
   */
  byte* getRow(size_t index) const;

  /**
   * Append a new row to the end of the storage, in case this means we
   * do not have sufficient space we will reallocate space
   * accordingly.
   */
  void appendRow(byte* tuple);


  void appendRows(const atable_ptr_t& rows);

  virtual void debugStructure(size_t level=0) const;

  
  ////////////////////////////////////////////////////////////////////////////////////////
  // Disabled Methodsw 
  virtual atable_ptr_t copy_structure(const field_list_t *fields = nullptr, 
                                      const bool reuse_dict = false, 
                                      const size_t initial_size = 0, 
                                      const bool with_containers = true, 
                                      const bool compressed = false) const {
    STORAGE_NOT_IMPLEMENTED(RawTable, copy_structure());
  }

  virtual atable_ptr_t copy_structure_modifiable(const field_list_t *fields = nullptr, 
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

  virtual size_t partitionWidth(const size_t slice) const {
    STORAGE_NOT_IMPLEMENTED(RawTable, partitionWidth());
  }

  virtual const AbstractTable::SharedDictionaryPtr& dictionaryAt(const size_t column, 
                                                          const size_t row = 0, 
                                                          const table_id_t table_id = 0) const { 
    STORAGE_NOT_IMPLEMENTED(RawTable, partitionWidth());
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

} } // namespace hyrise::storage

