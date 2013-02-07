// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_OPERATIONDATA_H_
#define SRC_LIB_ACCESS_OPERATIONDATA_H_

#include <helper/types.h>

#include <storage/storage_types.h>
#include <storage/Table.h>

#include <vector>

class AbstractHashTable;

typedef std::vector<hyrise::storage::c_atable_ptr_t >     table_list_t;
typedef std::vector<std::shared_ptr<AbstractHashTable> > hash_table_list_t;

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
    const std::vector<std::shared_ptr<T>> &ownElements,
    const std::vector<std::shared_ptr<T>> &otherElements,
    const bool retain = false);

public:
  OperationData()
    : tables(table_list_t()),
      hashTables(hash_table_list_t())
  {}

  /*
    Make the object self contained and release all inputs and outputs
  */
  virtual ~OperationData();

  /*!
   *  Add and retrieve elements of member lists.
   */

  void addHash(std::shared_ptr<AbstractHashTable> input);
  void add(hyrise::storage::c_atable_ptr_t input);

  hyrise::storage::c_atable_ptr_t getTable(const size_t index = 0) const;
  std::shared_ptr<AbstractHashTable> getHashTable(const size_t index = 0) const;

  /*!
   *  Member list size methods.
   */
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



#endif  // SRC_LIB_ACCESS_OPERATIONDATA_H_

