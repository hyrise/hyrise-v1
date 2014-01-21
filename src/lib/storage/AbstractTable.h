// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file AbstractTable.h
 *
 * Contains the class definition of AbstractTable.
 *
 */
#pragma once

#include <limits>
#include <memory>
#include <stdexcept>

#include <functional>
#include <vector>
#include <string>

#include "helper/types.h"
#include "helper/locking.h"
#include "helper/checked_cast.h"
#include "helper/unique_id.h"

#include "storage/AbstractResource.h"
#include "storage/BaseDictionary.h"
#include "storage/storage_types.h"

namespace hyrise {
namespace storage {

class ColumnMetadata;
class AbstractDictionary;
class AbstractAttributeVector;

typedef struct {
  std::shared_ptr<AbstractAttributeVector> attribute_vector;
  size_t attribute_offset;
} attr_vector_offset_t;

typedef std::vector<attr_vector_offset_t> attr_vectors_t;

class StorageException : public std::runtime_error {

public:

  explicit StorageException(const std::string &msg): std::runtime_error(msg)
  {}

};

class MissingColumnException : public std::runtime_error {
public:

  explicit MissingColumnException(const std::string &what): std::runtime_error(what)
  {}
};


/**
 * Abstract table is the magic base class for all data storages, it is used
 * in many different ways, in containers, plain tables, intermediate results
 * and more.
 */
class AbstractTable : public AbstractResource {

public:

  typedef std::shared_ptr<AbstractDictionary> SharedDictionaryPtr;

  /**
   * Copy the table's structure.
   * Returns a pointer to an AbstractTable with a copy of the current table's
   * structure, containing all fields specified in the first parameter or all
   * if left empty, as well as the current table's dictionary for those fields
   * in case reuse_dict is set to true.
   *
   * @param fields          List of fields to be copied (all if empty or nullptr).
   * @param reuse_dict      Also copy the table's dictionary (default=false).
   * @param initial_size    Initial size of the returned table (default=0).
   * @param with_containers Only used by derived classes.
   * @param compressed      Sets the compressed storage for the new table
   */
  virtual atable_ptr_t copy_structure(const field_list_t *fields = nullptr, bool reuse_dict = false, size_t initial_size = 0, bool with_containers = true, bool compressed = false) const;


  /**
   * Copy the table's structure modifiable.
   * Returns a pointer to an AbstractTable with a copy of the current table's
   * structure, containing all fields specified in the first parameter or all
   * if left empty, as well as the current table's dictionary-type for each
   * field, without values for future modification.
   *
   * @param fields          List of fields to be copied (all if empty or nullptr).
   * @param initial_size    Initial size of the returned table (default=0).
   * @param with_containers Only used by derived classes.
   */
  virtual atable_ptr_t copy_structure_modifiable(const field_list_t *fields = nullptr, size_t initial_size = 0, bool with_containers = true) const;

  typedef std::function<std::shared_ptr<AbstractDictionary>(DataType)> abstract_dictionary_callback;
  typedef std::function<std::shared_ptr<AbstractAttributeVector>(std::size_t)> abstract_attribute_vector_callback;

  /**
   * Copy structure with factory functions to replace attribute vectors and dictionaries
   * in `Table` instances. May need future enhancement for more fine-grained replacement
   * (i.e. per column or per main/delta or per partition).
   */
  virtual atable_ptr_t copy_structure(abstract_dictionary_callback, abstract_attribute_vector_callback) const { throw std::runtime_error("not implemented"); }

  /**
   * Get the value-IDs for a certain row.
   * Returns a ValueIdList object containing a vector of ValueIds for all
   * specified fields in a given row.
   *
   * @param row    Row from which to extract the ValueIDs.
   * @param fields List of respected fields (all if empty).
   */
  ValueIdList copyValueIds(size_t row, const field_list_t *fields = nullptr) const;


  /**
   * Get the metadata for a certain column.
   * Returns a pointer to a ColumnMetadata object for a specified column.
   * @note Must be implemented by any derived class!
   *
   * @param column   Column for which to return the metadata.
   * @param row      Row in that column (default=0).
   * @param table_id ID of the table from which to extract (default=0).
   */
  virtual const ColumnMetadata& metadataAt(size_t column, size_t row = 0, table_id_t table_id = 0) const = 0;

  /**
   * Returs a list of references to metadata of this table.
   *
   * The list is newly created for all calls to this method, but the
   * references stay the same. Thus calling this method incurrs a
   * linear cost to the width of the table.
   */
  metadata_vec_t metadata() const;

  /**
   * Get the dictionary for a certain column.
   * @note Must be implemented by any derived class!
   *
   * @param column   Column from which to extract the dictionary.
   * @param row      Row in that column (default=0).
   * @param table_id ID of the table from which to extract (default=0).
   */
  virtual const SharedDictionaryPtr& dictionaryAt(size_t column, size_t row = 0, table_id_t table_id = 0) const = 0;


  /**
   * Get the dictionary for a certain column by table ID.
   * @note Must be implemented by any derived class!
   *
   * @param column   Column from which to extract the dictionary.
   * @param table_id ID of the table from which to extract.
   */
  virtual const SharedDictionaryPtr& dictionaryByTableId(size_t column, table_id_t table_id) const = 0;


  /**
   * Get all dictionaries.
   *
   */
  std::vector<SharedDictionaryPtr> *dictionaries() const;


  /**
   * Sets the dictionary for a certain column.
   * @note Must be implemented by any derived class!
   *
   * @param dict     Dictionary to be used.
   * @param column   Column for which to set the dictionary.
   * @param row      Row in that column (default=0).
   * @param table_id ID of the table (default=0).
   */
  virtual void setDictionaryAt(SharedDictionaryPtr dict, size_t column, size_t row = 0, table_id_t table_id = 0) = 0;


  /**
   * Returns the type of the column.
   * @note Must be implemented by any derived class!
   *
   * @param column Column from which to extract the type.
   */
  DataType typeOfColumn(size_t column) const;


  /**
   * Returns the number of rows in the table
   * @note Must be implemented by any derived class!
   */
  virtual size_t size() const = 0;


  /**
   * Returns the number of columns.
   * @note Must be implemented by any derived class!
   */
  virtual size_t columnCount() const = 0;

  /**
   * Returns the number of a column by its name.
   *
   * @param column Name of the column as String.
   */
  field_t numberOfColumn(const std::string &column) const;

  /**
   * Returns the name of a column by its number.
   *
   * @param column Number of the column as numeric value.
   */
  std::string nameOfColumn(size_t column) const;


  /**
   * Returns the value-ID of a cell.
   * @note Must be implemented by any derived class!
   *
   * @param column Column number of the cell.
   * @param row    Row number of the cell.
   */
  virtual ValueId getValueId(size_t column, size_t row) const = 0;


  /**
   * Sets the value ID of a cell.
   * @note Should be implemented in derived classes or throws runtime error!
   *
   * @param column  Column number of the cell.
   * @param row     Row number of the cell.
   * @param valueId New value-ID of the cell.
   */
  virtual void setValueId(size_t column, size_t row, const ValueId valueId);


  /**
   * Reorganizes the bit vector of a certain column.
   * @warning Throws runtime error if not implemented!
   *
   * @param nr_of_values   Total number of values that must fit.
   */
  virtual void reserve(size_t nr_of_values);

  /**
   * Resize the table to the given number of rows based on the
   * parameter
   * @warning Throws runtime error if not implemented
   *
   * @param rows           The new number of rows in this table
   */
  virtual void resize(size_t rows);

  /**
   * Returns the number of partitions in this table.
   * @note Must be implemented by any derived class!
   */
  virtual unsigned partitionCount() const = 0;


  /**
   * Returns the width of a specified partition in number of attributes.
   * @note Must be implemented by any derived class!
   *
   * @param slice The slice of interest.
   */
  virtual size_t partitionWidth(size_t slice) const = 0;


  /**
   * Prints the table
   */
  virtual void print(size_t limit = (size_t) -1) const;

  /**
   * Returns the number of horizontal subtables.
   * @note Must be implemented by any derived class!
   */
  virtual table_id_t subtableCount() const = 0;


  /**
   * Templated method to retrieve the value-ID for a given value.
   *
   * @param column   Column containing the value.
   * @param value    The value.
   * @param create   Create the value if it is not existing (default=false)
   * @param table_id ID of the table containing the value (default=0).
   */
  template <typename T>
  inline ValueId getValueIdForValue(const size_t column, const T value, const bool create = false, const table_id_t table_id = 0) const {
    // FIXME horizontal containers will go down here, needs a row index, can be default 0
    const auto& map = checked_pointer_cast<BaseDictionary<T>>(dictionaryAt(column, 0, table_id));
    ValueId valueId;
    valueId.table = table_id;
    valueId.valueId = map->getValueId(value, create);
    return valueId;
  }

  /**
   * Templated method, checks whether or not a value is contained in a column.
   *
   * @param column   Column to check.
   * @param value    Value to look for.
   * @param table_id ID of the table (default=0).
   */
  template<typename T>
  inline bool valueExists(const field_t column, const T value, const table_id_t table_id = 0) const {
    const auto& map = checked_pointer_cast<BaseDictionary<T>>(dictionaryAt(column, 0, table_id));
    return map->valueExists(value);
  }

  /**
   * Templated base method for setting a value.
   *
   * @param column Column of the cell.
   * @param row    Row of the cell.
   * @param value  Value to be assigned to the cell.
   */
  template <typename T>
  void setValue(size_t column, size_t row, const T &value) {
    const auto& map = checked_pointer_cast<BaseDictionary<T>>(dictionaryAt(column, row));
    ValueId valueId;
    valueId.table = 0;
    valueId.valueId = map->insert(value);
    setValueId(column, row, valueId);
  }


  /**
   * Templated method for retrieving a value by its ID.
   *
   * @param column  Column containing the value.
   * @param valueId ID of the value to be returned.
   */
  template <typename T>
  inline T getValueForValueId(const field_t column, const ValueId valueId) const {
    typedef BaseDictionary<T> dict_t;
    if (valueId.table != 0) {
      return (static_cast<dict_t *>(dictionaryByTableId(column, valueId.table).get()))->getValueForValueId(valueId.valueId);
    } else {
      return (static_cast<dict_t *>(dictionaryAt(column, 0, valueId.table).get()))->getValueForValueId(valueId.valueId);
    }
  }


  /**
   * Templated method for retrieving a value from a cell.
   *
   * @param column Column of the cell.
   * @param row    Row of the cell.
   */
  template <typename T>
  T getValue(const field_t column, const size_t row) const {
    typedef BaseDictionary<T> dict_t;
    ValueId valueId = getValueId(column, row);
    if (valueId.table != 0) {
        return (static_cast<dict_t *>(dictionaryByTableId(column, valueId.table).get()))->getValueForValueId(valueId.valueId);
    } else {
        return (static_cast<dict_t *>(dictionaryAt(column, row, valueId.table).get()))->getValueForValueId(valueId.valueId);
    }
  }


  /**
   * Templated method for retrieving a value from a cell.
   *
   * @param column Column of the cell.
   * @param row    Row of the cell.
   */
  template <typename T>
  T getValue(const field_name_t &column_name, const size_t row) const {
    size_t column = numberOfColumn(column_name);
    return getValue<T>(column, row);
  }


  /**
   * Print the value.
   *
   * @param column Column of the cell containing the value.
   * @param row    Row of the cell containing the value.
   */
  std::string printValue(size_t column, size_t row) const;


  /**
   * Templated method for copying a value from another table.
   *
   * @param source  Table from which to copy the value.
   * @param src_col Column of the source cell.
   * @param src_row Row of the source cell.
   * @param dst_col Column of the target cell.
   * @param dst_row Row of the target cell.
   */
  template <typename T>
  void copyValueFrom(const c_atable_ptr_t& source, const size_t src_col, const size_t src_row, const size_t dst_col, const size_t dst_row) {
    T value = source->getValue<T>(src_col, src_row);
    setValue<T>(dst_col, dst_row, value);
  }


  /**
   * Copies a value from another table by column and row.
   *
   * @param source  Table from which to copy the value.
   * @param src_col Column of the source cell.
   * @param src_row Row of the source cell.
   * @param dst_col Column of the target cell.
   * @param dst_row Row of the target cell.
   */
  void copyValueFrom(const c_atable_ptr_t& source, size_t src_col, size_t src_row, size_t dst_col, size_t dst_row);


  /**
   * Copies a value from another table by column and value-ID.
   *
   * @param source  Table from which to copy the value.
   * @param src_col Column of the source cell.
   * @param vid     Value-ID in the source Column.
   * @param dst_col Column of the target cell.
   * @param dst_row Row of the target cell.
   */
  void copyValueFrom(const c_atable_ptr_t& source, size_t src_col, ValueId vid, size_t dst_col, size_t dst_row);


  /**
   * Copies a row from another table with or without values.
   *
   * @param source      Table from which to copy the value.
   * @param src_row     Row in the source table.
   * @param dst_row     Row in the target table.
   * @param copy_values Also copy the values (default=true).
   * @param use_memcpy  Use memcpy for the copying (default=true).
   */
  void copyRowFrom(const c_atable_ptr_t& source, size_t src_row, size_t dst_row, bool copy_values = true, bool use_memcpy = true);


  /**
   * Write the table data into a file as-is.
   *
   * @param filename Name of the file to be written to.
   */
  void write(const std::string &filename) const;



  /**
   * Test for equality of this table's content with another table's.
   *
   * @param other Table to compare content to.
   */
  bool contentEquals(const c_atable_ptr_t& other) const;


  /**
   * Create of copy of this table.
   * @note Must be implemented by any derived class!
   */
  virtual atable_ptr_t copy() const = 0;

  /**
  * get underlying attribute vectors for column
  *
  * This method returns a struct containing the reference to the attribute
  * vector and the offset of the attribut in this vector. This allows a direct
  * access to the memory and keeping the high-level data structures.
  */
  virtual const attr_vectors_t getAttributeVectors(size_t column) const;

  virtual void debugStructure(size_t level=0) const;

  unique_id getUuid() const;

  void setUuid(unique_id = unique_id());

 private:
  // Global unique identifier for this object
  unique_id _uuid;
};

} } // namespace hyrise::storage

