// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file AbstractTable.h
 *
 * Contains the class definition of AbstractTable.
 *
 */

#ifndef SRC_LIB_STORAGE_ABSTRACTTABLE_H_
#define SRC_LIB_STORAGE_ABSTRACTTABLE_H_

#include <limits>
#include <memory>
#include <stdexcept>

#include <vector>
#include <string>

#include "helper/types.h"

#include <storage/AbstractResource.h>
#include <storage/storage_types.h>
#include <storage/ColumnMetadata.h>
#include <storage/ValueIdMap.hpp>


template<typename Strategy, template<class T, class S> class Allocator> class Table;
class Store;

class MutableVerticalTable;
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

  friend class Store;

private:

  unsigned _generation;
  
public:

  typedef std::shared_ptr<AbstractDictionary> SharedDictionaryPtr;

  /**
   * Constructor.
   */
  AbstractTable() : _generation(0) {}


  /**
   * Destructor.
   */
  virtual ~AbstractTable() {}


  /**
   * Returns the generation value.
   */
  unsigned generation() const;


  /**
   * Sets the generation value.
   *
   * @param generation The generation.
   */
  void setGeneration(const unsigned generation);

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
  virtual hyrise::storage::atable_ptr_t copy_structure(const field_list_t *fields = nullptr, const bool reuse_dict = false, const size_t initial_size = 0, const bool with_containers = true, const bool compressed = false) const;


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
  hyrise::storage::atable_ptr_t copy_structure_modifiable(const field_list_t *fields = nullptr, const size_t initial_size = 0, const bool with_containers = true) const;


  /**
   * Get the value-IDs for a certain row.
   * Returns a ValueIdList object containing a vector of ValueIds for all
   * specified fields in a given row.
   *
   * @param row    Row from which to extract the ValueIDs.
   * @param fields List of respected fields (all if empty).
   */
  ValueIdList copyValueIds(const size_t row, const field_list_t *fields = nullptr) const;


  /**
   * Get the metadata for a certain column.
   * Returns a pointer to a ColumnMetadata object for a specified column.
   * @note Must be implemented by any derived class!
   *
   * @param column   Column for which to return the metadata.
   * @param row      Row in that column (default=0).
   * @param table_id ID of the table from which to extract (default=0).
   */
  virtual const ColumnMetadata *metadataAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0) const = 0;

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
  virtual const SharedDictionaryPtr& dictionaryAt(const size_t column, const size_t row = 0, const table_id_t table_id = 0, const bool of_delta = false) const = 0;


  /**
   * Get the dictionary for a certain column by table ID.
   * @note Must be implemented by any derived class!
   *
   * @param column   Column from which to extract the dictionary.
   * @param table_id ID of the table from which to extract.
   */
  virtual const SharedDictionaryPtr& dictionaryByTableId(const size_t column, const table_id_t table_id) const = 0;


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
  virtual void setDictionaryAt(SharedDictionaryPtr dict, const size_t column, const size_t row = 0, const table_id_t table_id = 0) = 0;


  /**
   * Returns the type of the column.
   * @note Must be implemented by any derived class!
   *
   * @param column Column from which to extract the type.
   */
  DataType typeOfColumn(const size_t column) const;


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
  std::string nameOfColumn(const size_t column) const;


  /**
   * Returns the value-ID of a cell.
   * @note Must be implemented by any derived class!
   *
   * @param column Column number of the cell.
   * @param row    Row number of the cell.
   */
  virtual ValueId getValueId(const size_t column, const size_t row) const = 0;


  /**
   * Sets the value ID of a cell.
   * @note Should be implemented in derived classes or throws runtime error!
   *
   * @param column  Column number of the cell.
   * @param row     Row number of the cell.
   * @param valueId New value-ID of the cell.
   */
  virtual void setValueId(const size_t column, const size_t row, const ValueId valueId);


  /**
   * Reorganizes the bit vector of a certain column.
   * @warning Throws runtime error if not implemented!
   *
   * @param nr_of_values   Total number of values that must fit.
   */
  virtual void reserve(const size_t nr_of_values);

  /**
   * Resize the table to the given number of rows based on the
   * parameter
   * @warning Throws runtime error if not implemented
   *
   * @param rows           The new number of rows in this table
   */
  virtual void resize(const size_t rows);

  /**
   * Returns the number of slices.
   * @note Must be implemented by any derived class!
   */
  virtual unsigned sliceCount() const = 0;


  /**
   * Returns a pointer to the memory area of a slice.
   * @note Must be implemented by any derived class!
   *
   * @param slice       The slice of interest.
   * @param row         Row in that slice.
   * @param _width
   */
  virtual void *atSlice(const size_t slice, const size_t row) const = 0;


  /**
   * Returns the width of a slice.
   * @note Must be implemented by any derived class!
   *
   * @param slice The slice of interest.
   */
  virtual size_t getSliceWidth(const size_t slice) const = 0;


  virtual size_t getSliceForColumn(const size_t column) const = 0;
  virtual size_t getOffsetInSlice(const size_t column) const = 0;


  /**
   * Prints the table
   */
  virtual void print(const size_t limit = (size_t) -1) const;

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
  inline ValueId getValueIdForValue(const size_t column, const T &value, const bool create = false, const table_id_t table_id = 0) const {

    // FIXME here should be some basic type checking, at least we should check with a better cast and catch the std::exception
    // FIXME horizontal containers will go down here, needs a row index, can be default 0

    const auto& map = std::dynamic_pointer_cast<BaseDictionary<T>>(dictionaryAt(column, 0, table_id));
    ValueId valueId;
    valueId.table = table_id;

    if (map->valueExists(value)) {
      valueId.valueId = map->getValueIdForValue(value);
    } else if (create) {
      valueId.valueId = map->addValue(value);
      /*if (map->isOrdered()) {
        throw std::runtime_error("Cannot insert value in an ordered dictionary");
      } else {
        
      }*/
    } else {
      // TODO: We should document that INT_MAX is an invalid document ID
      valueId.valueId = std::numeric_limits<value_id_t>::max();
    }

    return valueId;
  }


  /**
   * Templated method to retrieve the value-ID for a given value.
   * Calls dictionaryByTableId(...) instead of dictionaryAt(...)
   *
   * @param column   Column containing the value.
   * @param value    The value.
   * @param create   Create the value if it is not existing (default=false)
   * @param table_id ID of the table containing the value (default=0).
   */
  template <typename T>
  inline ValueId getValueIdForValueByTableId(const size_t column, const T value, const bool create = false, const table_id_t table_id = 0) const {

    // FIXME here should be some basic type checking, at least we should check with a better cast and catch the std::exception
    // FIXME horizontal containers will go down here, needs a row index, can be default 0

    const auto& map = std::dynamic_pointer_cast<BaseDictionary<T>>(dictionaryByTableId(column, table_id));
    ValueId valueId;
    valueId.table = table_id;

    if (map->valueExists(value)) {
      valueId.valueId = map->getValueIdForValue(value);
    } else if (create) {
      /*if (map->isOrdered()) {
        throw std::runtime_error("Cannot insert value in an ordered dictionary");
        }*/

      valueId.valueId = map->addValue(value);
    } else {
      valueId.valueId = INT_MAX;
    }

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
    const auto& map = std::dynamic_pointer_cast<BaseDictionary<T>>(dictionaryAt(column, 0, table_id));
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
  void setValue(const size_t column, const size_t row, const T &value) {
    ValueId valueId = getValueIdForValue(column, value, true);
    setValueId(column, row, valueId);

  }


  /**
   * Templated base method for setting a value using column name.
   *
   * @param column Name of the cell's column.
   * @param row    Row of the cell.
   * @param value  Value to be assigned to the cell.
   */
  template <typename T>
  void setValue(const field_name_t &column_name, const size_t row, const T value) {
    size_t column = numberOfColumn(column_name);
    setValue<T>(column, row, value);
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
      return std::static_pointer_cast<dict_t>(dictionaryByTableId(column, valueId.table))->getValueForValueId(valueId.valueId);
    } else {
      return std::static_pointer_cast<dict_t>(dictionaryAt(column, 0, valueId.table))->getValueForValueId(valueId.valueId);
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
      return std::static_pointer_cast<dict_t>(dictionaryByTableId(column, valueId.table))->getValueForValueId(valueId.valueId);
    } else {
      return std::static_pointer_cast<dict_t>(dictionaryAt(column, row, valueId.table))->getValueForValueId(valueId.valueId);
    }
    //return std::static_pointer_cast<dict_t>(dictionaryAt(column, row, valueId.table))->getValueForValueId(valueId.valueId);
    //return getValueForValueId<T>(column, valueId);
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
  std::string printValue(const size_t column, const size_t row) const;


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
  void copyValueFrom(const hyrise::storage::c_atable_ptr_t& source, const size_t src_col, const size_t src_row, const size_t dst_col, const size_t dst_row) {
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
  void copyValueFrom(const hyrise::storage::c_atable_ptr_t& source, const size_t src_col, const size_t src_row, const size_t dst_col, const size_t dst_row);


  /**
   * Copies a value from another table by column and value-ID.
   *
   * @param source  Table from which to copy the value.
   * @param src_col Column of the source cell.
   * @param vid     Value-ID in the source Column.
   * @param dst_col Column of the target cell.
   * @param dst_row Row of the target cell.
   */
  void copyValueFrom(const hyrise::storage::c_atable_ptr_t& source, const size_t src_col, const ValueId vid, const size_t dst_col, const size_t dst_row);


  /**
   * Copies a row from another table with or without values.
   *
   * @param source      Table from which to copy the value.
   * @param src_row     Row in the source table.
   * @param dst_row     Row in the target table.
   * @param copy_values Also copy the values (default=true).
   * @param use_memcpy  Use memcpy for the copying (default=true).
   */
  void copyRowFrom(const hyrise::storage::c_atable_ptr_t& source, const size_t src_row, const size_t dst_row, const bool copy_values = true, const bool use_memcpy = true);


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
  bool contentEquals(const hyrise::storage::c_atable_ptr_t& other) const;


  /**
   * Create of copy of this table.
   * @note Must be implemented by any derived class!
   */
  virtual hyrise::storage::atable_ptr_t copy() const = 0;

  /// get underlying attribute vectors for column
  virtual const attr_vectors_t getAttributeVectors(size_t column) const;

  virtual void debugStructure(size_t level=0) const;
};

#endif  // SRC_LIB_STORAGE_ABSTRACTTABLE_H_

