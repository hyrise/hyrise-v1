// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_OPERATIONDATA_H_
#define SRC_LIB_ACCESS_OPERATIONDATA_H_

#include <helper/types.h>

#include <storage/storage_types.h>
#include <storage/Table.h>

#include <vector>

class AbstractHashTable;

typedef std::vector<hyrise::storage::c_atable_ptr_t>     table_list_t;
typedef std::vector<hyrise::storage::c_ahashtable_ptr_t> hash_table_list_t;

namespace hyrise {
namespace access {

/*!
 *  Data container for operator input/output handling.
 */
class OperationData {
protected:
  table_list_t        tables;
  hash_table_list_t   hashTables;

  /*!
   *  Templated merge used on member lists. Merge all elements of given
   *  otherElements into ownElements, including retain counting.
   */
  template <class T>
  void merge(
    const std::vector<std::shared_ptr<const T>> &ownElements,
    const std::vector<std::shared_ptr<const T>> &otherElements,
    const bool retain = false);

public:
  void addHash(storage::c_ahashtable_ptr_t input);
  void add(storage::c_atable_ptr_t input);
  void setHash(storage::c_ahashtable_ptr_t input, size_t index);
  void setTable(storage::c_atable_ptr_t input, size_t index);
  
  storage::c_atable_ptr_t getTable(const size_t index = 0) const;
  storage::c_ahashtable_ptr_t getHashTable(const size_t index = 0) const;
  
  size_t numberOfTables() const;
  size_t numberOfHashTables() const;
  bool emptyTables() const;
  bool emptyHashTables() const;

  /*!
   *  Return member lists for direct access.
   */
  const table_list_t & getTables() const;
  table_list_t &getTables();
  hash_table_list_t &getHashTables();

  /*!
   *  Merge all elements of all member lists of given other objects into
   *  own ones, including retain counting.
   */
  void mergeWith(
    OperationData &other,
    const bool retain = false);
};

}}

#endif  // SRC_LIB_ACCESS_OPERATIONDATA_H_

